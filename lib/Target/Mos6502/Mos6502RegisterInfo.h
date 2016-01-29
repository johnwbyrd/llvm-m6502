// TODO: header stuff

#ifndef LLVM_LIB_TARGET_MOS6502_MOS6502REGISTERINFO_H
#define LLVM_LIB_TARGET_MOS6502_MOS6502REGISTERINFO_H

#define GET_REGINFO_HEADER
#include "Mos6502GenRegisterInfo.inc"

namespace llvm {

class Mos6502RegisterInfo : public Mos6502GenRegisterInfo {
public:
  Mos6502RegisterInfo();
  
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;

  void eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  unsigned getFrameRegister(const MachineFunction &MF) const override;
};

} // end namespace llvm

#endif
