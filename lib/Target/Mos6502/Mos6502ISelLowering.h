// TODO: header stuff

#ifndef LLVM_LIB_TARGET_MOS6502_MOS6502ISELLOWERING_H
#define LLVM_LIB_TARGET_MOS6502_MOS6502ISELLOWERING_H

#include "llvm/Target/TargetLowering.h"

namespace llvm {
class Mos6502Subtarget;

class Mos6502TargetLowering : public TargetLowering {
public:
  Mos6502TargetLowering(const TargetMachine &TM,
                        const Mos6502Subtarget &Subtarget);
};

} // end namespace llvm

#endif
