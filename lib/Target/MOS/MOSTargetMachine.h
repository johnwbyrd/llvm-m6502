//===-- MOSTargetMachine.h - Define TargetMachine for MOS ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the MOS specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MOS_MOSTARGETMACHINE_H
#define LLVM_LIB_TARGET_MOS_MOSTARGETMACHINE_H

#include "MOSInstrInfo.h"
#include "MOSSubtarget.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class MOSTargetMachine : public LLVMTargetMachine {
  std::unique_ptr<TargetLoweringObjectFile> TLOF;
  MOSSubtarget Subtarget;
  bool is64Bit;
  mutable StringMap<std::unique_ptr<MOSSubtarget>> SubtargetMap;
public:
  MOSTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                     StringRef FS, const TargetOptions &Options,
                     Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                     CodeGenOpt::Level OL, bool JIT, bool is64bit);
  ~MOSTargetMachine() override;

  const MOSSubtarget *getSubtargetImpl() const { return &Subtarget; }
  const MOSSubtarget *getSubtargetImpl(const Function &) const override;

  // Pass Pipeline Configuration
  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;
  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

/// MOS 32-bit target machine
///
class MOSV8TargetMachine : public MOSTargetMachine {
  virtual void anchor();
public:
  MOSV8TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                       CodeGenOpt::Level OL, bool JIT);
};

/// MOS 64-bit target machine
///
class MOSV9TargetMachine : public MOSTargetMachine {
  virtual void anchor();
public:
  MOSV9TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                       CodeGenOpt::Level OL, bool JIT);
};

class MOSelTargetMachine : public MOSTargetMachine {
  virtual void anchor();

public:
  MOSelTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options,
                       Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
                       CodeGenOpt::Level OL, bool JIT);
};

} // end namespace llvm

#endif
