// TODO: header stuff

#include "Mos6502.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheMos6502Target;

extern "C" void LLVMInitializeMos6502TargetInfo() {
  RegisterTarget<Triple::mos6502> X(TheMos6502Target, "mos6502", "MOS 6502");
}
