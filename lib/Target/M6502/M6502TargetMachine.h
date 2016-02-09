// TODO: license stuff

#ifndef LLVM_LIB_TARGET_M6502_M6502TARGETMACHINE_H
#define LLVM_LIB_TARGET_M6502_M6502TARGETMACHINE_H

#include "M6502TargetObjectFile.h"
#include "M6502Subtarget.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

class M6502TargetMachine : public LLVMTargetMachine {
  std::unique_ptr<M6502TargetObjectFile> TLOF;
  M6502Subtarget Subtarget;

public:
  M6502TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                     StringRef FS, const TargetOptions &Options, Reloc::Model RM,
                     CodeModel::Model CM, CodeGenOpt::Level OL);

  const M6502Subtarget *getSubtargetImpl(const Function &) const override {
    return &Subtarget;
  }

  TargetPassConfig *createPassConfig(PassManagerBase &PM) override;

  TargetLoweringObjectFile *getObjFileLowering() const override {
    return TLOF.get();
  }
};

} // end namespace llvm

#endif
