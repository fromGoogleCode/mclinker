/*****************************************************************************
 *   The MC Linker Project, Copyright (C), 2011 -                            *
 *   Embedded and Web Computing Lab, National Taiwan University              *
 *   MediaTek, Inc.                                                          *
 *                                                                           *
 *   Anders Cheng <Anders.Cheng@mediatek.com>                                *
 ****************************************************************************/
#ifndef X86_ANDROIDSECTLINKER_H
#define X86_ANDROIDSECTLINKER_H
#ifdef ENABLE_UNITTEST
#include <gtest.h>
#endif
#include <mcld/Target/AndroidSectLinker.h>

namespace mcld
{

class MCLDInfo;

/** \class X86AndroidSectLinker
 *  \brief X86AndroidSectLinker sets up the environment for linking.
 *
 *  \see
 *  \author Anders Cheng <Anders.Cheng@mediatek.com>
 */
class X86AndroidSectLinker : public AndroidSectLinker
{
public:
  X86AndroidSectLinker(const std::string &pInputFilename,
                       const std::string &pOutputFilename,
                       unsigned int pOutputLinkType,
                       MCLDInfo& pLDInfo,
                       mcld::TargetLDBackend &pLDBackend);

  ~X86AndroidSectLinker();
};

} // namespace of mcld

#endif

