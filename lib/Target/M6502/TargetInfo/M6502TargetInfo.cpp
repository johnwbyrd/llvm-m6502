// TODO: header stuff

#include "M6502.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheM6502Target;

extern "C" void LLVMInitializeM6502TargetInfo() {
  RegisterTarget<Triple::m6502> X(TheM6502Target, "m6502", "MOS 6502");
}
