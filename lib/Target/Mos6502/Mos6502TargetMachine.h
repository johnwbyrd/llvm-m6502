// TODO: license stuff

#ifndef LLVM_LIB_TARGET_MOS6502_MOS6502TARGETMACHINE_H
#define LLVM_LIB_TARGET_MOS6502_MOS6502TARGETMACHINE_H

#include "llvm/Target/TargetMachine.h"

namespace llvm {

class Mos6502TargetMachine : public LLVMTargetMachine {

public:
  Mos6502TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                       StringRef FS, const TargetOptions &Options, Reloc::Model RM,
                       CodeModel::Model CM, CodeGenOpt::Level OL);
};

} // end namespace llvm

#endif
