// TODO: Header stuff

#ifndef LLVM_LIB_TARGET_M6502_M6502_H
#define LLVM_LIB_TARGET_M6502_M6502_H

#include "MCTargetDesc/M6502MCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class FunctionPass;
class M6502TargetMachine;

// ISel passes.
FunctionPass *createM6502ISelDag(M6502TargetMachine &TM,
                                 CodeGenOpt::Level OptLevel);

} // end namespace llvm

#endif
