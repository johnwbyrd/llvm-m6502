// TODO: header stuff

#ifndef LLVM_LIB_TARGET_MOS6502_MCTARGETDESC_MOS6502MCTARGETDESC_H
#define LLVM_LIB_TARGET_MOS6502_MCTARGETDESC_MOS6502MCTARGETDESC_H

namespace llvm {
class Target;

extern Target TheMos6502Target;

} // End llvm namespace

#define GET_REGINFO_ENUM
#include "Mos6502GenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "Mos6502GenInstrInfo.inc"

#endif
