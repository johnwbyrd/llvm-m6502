//===-- MOSTargetObjectFile.h - MOS Object Info -------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MOS_MOSTARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_MOS_MOSTARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {

class MCContext;
class TargetMachine;

class MOSELFTargetObjectFile : public TargetLoweringObjectFileELF {
public:
  MOSELFTargetObjectFile() :
    TargetLoweringObjectFileELF()
  {}

  void Initialize(MCContext &Ctx, const TargetMachine &TM) override;

  const MCExpr *getTTypeGlobalReference(const GlobalValue *GV,
                                        unsigned Encoding,
                                        const TargetMachine &TM,
                                        MachineModuleInfo *MMI,
                                        MCStreamer &Streamer) const override;
};

} // end namespace llvm

#endif
