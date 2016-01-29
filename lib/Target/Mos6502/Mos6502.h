// TODO: Header stuff

#ifndef LLVM_LIB_TARGET_MOS6502_MOS6502_H
#define LLVM_LIB_TARGET_MOS6502_MOS6502_H

#include "MCTargetDesc/Mos6502MCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {
class FunctionPass;
class Mos6502TargetMachine;

FunctionPass *createMos6502ISelDag(Mos6502TargetMachine &TM,
                                   CodeGenOpt::Level OptLevel);

} // end namespace llvm

#endif
