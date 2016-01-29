// TODO: header stuff

#include "Mos6502RegisterInfo.h"
#include "Mos6502.h"
#include "Mos6502Subtarget.h"
#include "llvm/CodeGen/MachineFunction.h"

using namespace llvm;

#define GET_REGINFO_TARGET_DESC
#include "Mos6502GenRegisterInfo.inc"

// FIXME: What does first param mean?
// RA? Return Address?
Mos6502RegisterInfo::Mos6502RegisterInfo() : Mos6502GenRegisterInfo(0) {}

const MCPhysReg *
Mos6502RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  static const MCPhysReg CalleeSavedRegs[] = { 0 }; // TODO
  return CalleeSavedRegs;
}

BitVector
Mos6502RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  // TODO
  return Reserved;
}

void
Mos6502RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                         int SPAdj, unsigned FIOperandNum,
                                         RegScavenger *RS) const {
  // TODO
}

unsigned
Mos6502RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return 0; // TODO
}
