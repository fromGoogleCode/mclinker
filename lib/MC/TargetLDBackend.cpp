//===-- TargetAsmBackend.cpp - Target Assembly Backend ---------------------==//
//
//                     The MCLinker Project
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "mcld/Target/TargetLDBackend.h"
using namespace llvm;

TargetLDBackend::TargetLDBackend()
  : HasReliableSymbolDifference(false)
{
}

TargetLDBackend::~TargetLDBackend() {
}

const MCFixupKindInfo &
TargetLDBackend::getFixupKindInfo(MCFixupKind Kind) const {
  static const MCFixupKindInfo Builtins[] = {
    { "FK_Data_1", 0, 8, 0 },
    { "FK_Data_2", 0, 16, 0 },
    { "FK_Data_4", 0, 32, 0 },
    { "FK_Data_8", 0, 64, 0 },
    { "FK_PCRel_1", 0, 8, MCFixupKindInfo::FKF_IsPCRel },
    { "FK_PCRel_2", 0, 16, MCFixupKindInfo::FKF_IsPCRel },
    { "FK_PCRel_4", 0, 32, MCFixupKindInfo::FKF_IsPCRel },
    { "FK_PCRel_8", 0, 64, MCFixupKindInfo::FKF_IsPCRel }
  };
  
  assert((size_t)Kind <= sizeof(Builtins) / sizeof(Builtins[0]) &&
         "Unknown fixup kind");
  return Builtins[Kind];
}
