// TODO: header stuff

#include "M6502RegisterInfo.h"
#include "M6502.h"
#include "M6502Subtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
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

const TargetRegisterClass *
M6502RegisterInfo::getLargestLegalSuperClass(const TargetRegisterClass *RC,
                                             const MachineFunction &) const {
  if (M6502::DREGSRegClass.hasSubClassEq(RC)) {
    return &M6502::DREGSRegClass;
  }

  if (M6502::GPR8RegClass.hasSubClassEq(RC)) {
    return &M6502::GPR8RegClass;
  }

  // NOTE: Flag registers cannot be copied or spilled. Therefore, they are not
  // considered here.

  return RC;
}

void
M6502RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                       int SPAdj, unsigned FIOperandNum,
                                       RegScavenger *RS) const {
  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &FrameInfo = MF.getFrameInfo();
  const M6502FrameLowering *TFI = getFrameLowering(MF);

  int Index = MI.getOperand(FIOperandNum).getIndex();
  MI.getOperand(FIOperandNum).ChangeToImmediate(Index); // ????
  //MI.getOperand(FIOperandNum).ChangeToImmediate(FrameInfo.getObjectOffset(Index));
}

unsigned
M6502RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return 0; // TODO
}

void M6502RegisterInfo::splitReg(unsigned Reg,
  unsigned &LoReg,
  unsigned &HiReg) const {
  assert(M6502::DREGSRegClass.contains(Reg) && "can only split 16-bit registers");

  LoReg = getSubReg(Reg, M6502::sub_lo);
  HiReg = getSubReg(Reg, M6502::sub_hi);
}
