// TODO: header stuff

#ifndef LLVM_LIB_TARGET_M6502_MCTARGETDESC_M6502MCTARGETDESC_H
#define LLVM_LIB_TARGET_M6502_MCTARGETDESC_M6502MCTARGETDESC_H

#include "llvm/MC/MCInstrDesc.h"

namespace llvm {
class Target;

extern Target TheM6502Target;

namespace M6502 {
enum OperandType {
  // Custom address operand
  OPERAND_M6502_ADDRESS = MCOI::OPERAND_FIRST_TARGET, // XXX: not used
};
}

} // End llvm namespace

#define GET_REGINFO_ENUM
#include "M6502GenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#include "M6502GenInstrInfo.inc"

#endif
