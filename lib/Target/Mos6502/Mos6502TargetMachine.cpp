// TODO: license stuff

#include "Mos6502TargetMachine.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

namespace llvm {
extern Target TheMos6502Target;
}

extern "C" void LLVMInitializeMos6502Target() {
  RegisterTargetMachine<Mos6502TargetMachine> X(TheMos6502Target);
}

Mos6502TargetMachine::Mos6502TargetMachine(const Target &T, const Triple &TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           Reloc::Model RM, CodeModel::Model CM,
                                           CodeGenOpt::Level OL)
    : LLVMTargetMachine(T, "", TT, CPU, FS, Options, RM, CM, OL) {
      // TODO: Data layout string
  // TODO
}
