//===- DiagnosticEngine.cpp -----------------------------------------------===//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include <mcld/LD/DiagnosticEngine.h>
#include <mcld/LD/DiagnosticPrinter.h>
#include <mcld/LD/DiagnosticLineInfo.h>
#include <mcld/MC/MCLDInfo.h>

using namespace mcld;

//==========================
// DiagnosticEngine
DiagnosticEngine::DiagnosticEngine(const MCLDInfo& pLDInfo,
                                   DiagnosticLineInfo* pLineInfo,
                                   DiagnosticPrinter* pPrinter,
                                   bool pShouldOwnPrinter)
  : m_LDInfo(pLDInfo),
    m_pLineInfo(pLineInfo),
    m_pPrinter(pPrinter),
    m_OwnPrinter(pShouldOwnPrinter) {
}

DiagnosticEngine::~DiagnosticEngine()
{
  if (m_OwnPrinter && m_pPrinter != NULL)
    delete m_pPrinter;

  if (NULL != m_pLineInfo)
    delete m_pLineInfo;
}

void DiagnosticEngine::setPrinter(DiagnosticPrinter& pPrinter,
                                  bool pShouldOwnPrinter)
{
  if (m_OwnPrinter && m_pPrinter != NULL)
    delete m_pPrinter;
  m_pPrinter = &pPrinter;
  m_OwnPrinter = pShouldOwnPrinter;
}
