// TODO: header stuff

#include "M6502RegisterInfo.h"
#include "M6502.h"
#include "M6502Subtarget.h"
#include "llvm/CodeGen/MachineFunction.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "M6502GenRegisterInfo.inc"

// FIXME: What does first param mean?
// RA? Return Address?
M6502RegisterInfo::M6502RegisterInfo() : M6502GenRegisterInfo(0) {}

const MCPhysReg *
M6502RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  static const MCPhysReg CalleeSavedRegs[] = { 0 }; // TODO
  return CalleeSavedRegs;
}

BitVector
M6502RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  // TODO
  return Reserved;
}

void
M6502RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                       int SPAdj, unsigned FIOperandNum,
                                       RegScavenger *RS) const {
  // TODO
}

unsigned
M6502RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return 0; // TODO
}
