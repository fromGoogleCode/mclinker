//===- MCLinker.cpp -------------------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the MCLinker class.
//
//===----------------------------------------------------------------------===//
#include <mcld/CodeGen/MCLinker.h>

#include <mcld/Config/Config.h>
#include <mcld/Module.h>
#include <mcld/Support/FileHandle.h>
#include <mcld/MC/InputTree.h>
#include <mcld/MC/InputBuilder.h>
#include <mcld/MC/InputFactory.h>
#include <mcld/Object/ObjectLinker.h>
#include <mcld/Support/FileSystem.h>
#include <mcld/Support/MsgHandling.h>
#include <mcld/Support/FileHandle.h>
#include <mcld/Support/raw_ostream.h>
#include <mcld/Support/MemoryArea.h>
#include <mcld/Support/MemoryAreaFactory.h>
#include <mcld/Support/DerivedPositionDependentOptions.h>
#include <mcld/Target/TargetLDBackend.h>
#include <mcld/CodeGen/SectLinkerOption.h>

#include <llvm/Module.h>

#include <algorithm>
#include <stack>
#include <string>

using namespace mcld;
using namespace llvm;

//===----------------------------------------------------------------------===//
// Forward declarations
//===----------------------------------------------------------------------===//
char MCLinker::m_ID = 0;
static bool CompareOption(const PositionDependentOption* X,
                          const PositionDependentOption* Y);

//===----------------------------------------------------------------------===//
// MCLinker
//===----------------------------------------------------------------------===//
MCLinker::MCLinker(SectLinkerOption &pOption,
                   TargetLDBackend& pLDBackend,
                   mcld::Module& pModule,
                   MemoryArea& pOutput)
  : MachineFunctionPass(m_ID),
    m_pOption(&pOption),
    m_pLDBackend(&pLDBackend),
    m_Module(pModule),
    m_Output(pOutput),
    m_pObjLinker(NULL),
    m_InputFactory(MCLD_NUM_OF_INPUTS, pOption.config().attribute()),
    m_MemAreaFactory(MCLD_NUM_OF_INPUTS),
    m_ContextFactory(MCLD_NUM_OF_INPUTS),
    m_pBuilder(NULL) {
}

MCLinker::~MCLinker()
{
  delete m_pObjLinker;

  // FIXME: current implementation can not change the order of deleting
  // ObjectLinker and TargetLDBackend. Because the deletion of relocation list
  // in FragmentLinker (FragmentLinker is deleted by ObjectLinker) depends on
  // RelocationFactory in TargetLDBackend

  // Instance of TargetLDBackend was created outside and is not managed by
  // MCLinker. It should not be destroyed here and by MCLinker. However, in
  // order to follow the LLVM convention - that is, the pass manages all the
  // objects it used during the processing, we destroy the object of
  // TargetLDBackend here.
  delete m_pLDBackend;

  delete m_pBuilder;
}

bool MCLinker::doInitialization(llvm::Module &pM)
{
  LinkerConfig &config = m_pOption->config();

  m_pBuilder = new InputBuilder(config,
                                m_InputFactory,
                                m_MemAreaFactory,
                                m_ContextFactory);

  // ----- convert position dependent options into tree of input files  ----- //
  PositionDependentOptions &PosDepOpts = m_pOption->pos_dep_options();
  std::stable_sort(PosDepOpts.begin(), PosDepOpts.end(), CompareOption);
  initializeInputTree(PosDepOpts);
  initializeInputOutput();

  // Now, all input arguments are prepared well, send it into ObjectLinker
  m_pObjLinker = new ObjectLinker(config,
                                  *m_pLDBackend,
                                  m_Module,
                                  *m_pBuilder);

  return false;
}

bool MCLinker::doFinalization(llvm::Module &pM)
{
  const LinkerConfig &config = m_pOption->config();

  // 2. - initialize FragmentLinker
  if (!m_pObjLinker->initFragmentLinker())
    return true;

  // 3. - initialize output's standard sections
  if (!m_pObjLinker->initStdSections())
    return true;

  // 4. - normalize the input tree
  m_pObjLinker->normalize();

  if (config.options().trace()) {
    static int counter = 0;
    mcld::outs() << "** name\ttype\tpath\tsize (" << m_Module.getInputTree().size() << ")\n";
    InputTree::const_dfs_iterator input, inEnd = m_Module.getInputTree().dfs_end();
    for (input=m_Module.getInputTree().dfs_begin(); input!=inEnd; ++input) {
      mcld::outs() << counter++ << " *  " << (*input)->name();
      switch((*input)->type()) {
      case Input::Archive:
        mcld::outs() << "\tarchive\t(";
        break;
      case Input::Object:
        mcld::outs() << "\tobject\t(";
        break;
      case Input::DynObj:
        mcld::outs() << "\tshared\t(";
        break;
      case Input::Script:
        mcld::outs() << "\tscript\t(";
        break;
      case Input::External:
        mcld::outs() << "\textern\t(";
        break;
      default:
        unreachable(diag::err_cannot_trace_file) << (*input)->type()
                                                 << (*input)->name()
                                                 << (*input)->path();
      }
      mcld::outs() << (*input)->path() << ")\n";
    }
  }

  // 5. - check if we can do static linking and if we use split-stack.
  if (!m_pObjLinker->linkable())
    return true;


  // 6. - merge all sections
  if (!m_pObjLinker->mergeSections())
    return true;

  // 7. - add standard symbols and target-dependent symbols
  // m_pObjLinker->addUndefSymbols();
  if (!m_pObjLinker->addStandardSymbols() ||
      !m_pObjLinker->addTargetSymbols())
    return true;

  // 8. - read all relocation entries from input files
  m_pObjLinker->readRelocations();

  // 9. - pre-layout
  m_pObjLinker->prelayout();

  // 10. - linear layout
  m_pObjLinker->layout();

  // 10.b - post-layout (create segment, instruction relaxing)
  m_pObjLinker->postlayout();

  // 11. - finalize symbol value
  m_pObjLinker->finalizeSymbolValue();

  // 12. - apply relocations
  m_pObjLinker->relocation();

  // 13. - write out output
  m_pObjLinker->emitOutput(m_Output);

  // 14. - post processing
  m_pObjLinker->postProcessing(m_Output);
  return false;
}

bool MCLinker::runOnMachineFunction(MachineFunction& pF)
{
  // basically, linkers do nothing during function is generated.
  return false;
}

void MCLinker::initializeInputOutput()
{
  // -----  initialize input files  ----- //
  InputTree::dfs_iterator input, inEnd = m_Module.getInputTree().dfs_end();
  for (input = m_Module.getInputTree().dfs_begin(); input!=inEnd; ++input) {
    // already got type - for example, bitcode
    if ((*input)->type() == Input::Script ||
        (*input)->type() == Input::Object ||
        (*input)->type() == Input::DynObj  ||
        (*input)->type() == Input::Archive)
      continue;

    if (!m_pBuilder->setMemory(**input, FileHandle::ReadOnly)) {
      error(diag::err_cannot_open_input) << (*input)->name() << (*input)->path();
      return;
    }

    m_pBuilder->setContext(**input);
  }
}

void MCLinker::initializeInputTree(const PositionDependentOptions &pPosDepOptions)
{
  if (pPosDepOptions.empty())
    fatal(diag::err_no_inputs);

  LinkerConfig &config= m_pOption->config();
  PositionDependentOptions::const_iterator option = pPosDepOptions.begin();
  if (1 == pPosDepOptions.size() &&
      ((*option)->type() != PositionDependentOption::INPUT_FILE &&
       (*option)->type() != PositionDependentOption::NAMESPEC) &&
       (*option)->type() != PositionDependentOption::BITCODE) {
    // if we only have one positional options, and the option is
    // not an input file, then emit error message.
    fatal(diag::err_no_inputs);
  }

  // -----  Input tree insertion algorithm  ----- //
  //   The type of the previsou node indicates the direction of the current
  //   insertion.
  //
  //     root   : the parent node who being inserted.
  //     mover  : the direcion of current movement.
  //
  //   for each positional options:
  //     insert the options in current root.
  //     calculate the next movement

  // Initialization
  InputTree::Mover *move = &InputTree::Downward;
  InputTree::iterator root = m_Module.getInputTree().root();
  PositionDependentOptions::const_iterator optionEnd = pPosDepOptions.end();
  std::stack<InputTree::iterator> returnStack;

  while (option != optionEnd ) {

    switch ((*option)->type()) {
      /** bitcode **/
      case PositionDependentOption::BITCODE: {

        const BitCodeOption *bitcode_option =
            static_cast<const BitCodeOption*>(*option);

        // threat bitcode as an external IR in this version.
        Input* input = m_InputFactory.produce(bitcode_option->path()->native(),
                                              *(bitcode_option->path()),
                                              Input::External);

        m_Module.getInputTree().insert(root, *move, *input);

        config.bitcode().setPath(*bitcode_option->path());
        config.bitcode().setPosition(bitcode_option->position());

        // move root on the new created node.
        move->move(root);

        // the next file is appended after bitcode file.
        move = &InputTree::Afterward;
        break;
      }

      /** input object file **/
      case PositionDependentOption::INPUT_FILE: {
        const InputFileOption *input_file_option =
            static_cast<const InputFileOption*>(*option);

        Input* input = m_InputFactory.produce(
                                           input_file_option->path()->native(),
                                           *(input_file_option->path()));

        m_Module.getInputTree().insert(root, *move, *input);

        // move root on the new created node.
        move->move(root);

        // the next file is appended after object file.
        move = &InputTree::Afterward;
        break;
      }

    /** -lnamespec **/
    case PositionDependentOption::NAMESPEC: {
      sys::fs::Path* path = NULL;
      const NamespecOption *namespec_option =
          static_cast<const NamespecOption*>(*option);

      // find out the real path of the namespec.
      if (config.attribute().constraint().isSharedSystem()) {
        // In the system with shared object support, we can find both archive
        // and shared object.

        if (m_InputFactory.attr().isStatic()) {
          // with --static, we must search an archive.
          path = config.options().directories().find(namespec_option->namespec(),
                                                   Input::Archive);
        }
        else {
          // otherwise, with --Bdynamic, we can find either an archive or a
          // shared object.
          path = config.options().directories().find(namespec_option->namespec(),
                                                   Input::DynObj);
        }
      }
      else {
        // In the system without shared object support, we only look for an
        // archive.
        path = config.options().directories().find(namespec_option->namespec(),
                                                 Input::Archive);
      }

      if (NULL == path)
        fatal(diag::err_cannot_find_namespec) << namespec_option->namespec();

      Input* input = m_InputFactory.produce(namespec_option->namespec(), *path);
      m_Module.getInputTree().insert(root, *move, *input);

      // iterate root on the new created node.
      move->move(root);

      // the file after a namespec must be appended afterward.
      move = &InputTree::Afterward;
      break;
    }

    /** start group **/
    case PositionDependentOption::START_GROUP:
      if (!returnStack.empty())
        fatal(diag::fatal_forbid_nest_group);
      m_Module.getInputTree().enterGroup(root, *move);
      move->move(root);
      returnStack.push(root);
      move = &InputTree::Downward;
      break;
    /** end group **/
    case PositionDependentOption::END_GROUP:
      root = returnStack.top();
      returnStack.pop();
      move = &InputTree::Afterward;
      break;
    case PositionDependentOption::WHOLE_ARCHIVE:
      m_InputFactory.attr().setWholeArchive();
      break;
    case PositionDependentOption::NO_WHOLE_ARCHIVE:
      m_InputFactory.attr().unsetWholeArchive();
      break;
    case PositionDependentOption::AS_NEEDED:
      m_InputFactory.attr().setAsNeeded();
      break;
    case PositionDependentOption::NO_AS_NEEDED:
      m_InputFactory.attr().unsetAsNeeded();
      break;
    case PositionDependentOption::ADD_NEEDED:
      m_InputFactory.attr().setAddNeeded();
      break;
    case PositionDependentOption::NO_ADD_NEEDED:
      m_InputFactory.attr().unsetAddNeeded();
      break;
    case PositionDependentOption::BSTATIC:
      m_InputFactory.attr().setStatic();
      break;
    case PositionDependentOption::BDYNAMIC:
      m_InputFactory.attr().setDynamic();
      break;
    default:
      fatal(diag::err_cannot_identify_option) << (*option)->position()
                                              << (uint32_t)(*option)->type();
    } // end of switch
    ++option;
  } // end of while

  if (!returnStack.empty()) {
    report_fatal_error("no matched --start-group and --end-group");
  }
}

//===----------------------------------------------------------------------===//
// Non-member functions
static bool CompareOption(const PositionDependentOption* X,
                          const PositionDependentOption* Y)
{
  return (X->position() < Y->position());
}

