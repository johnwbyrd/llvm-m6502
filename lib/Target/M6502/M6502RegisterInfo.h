// TODO: header stuff

#ifndef LLVM_LIB_TARGET_M6502_M6502REGISTERINFO_H
#define LLVM_LIB_TARGET_M6502_M6502REGISTERINFO_H

#define GET_REGINFO_HEADER
#include "M6502GenRegisterInfo.inc"

namespace llvm {

class M6502RegisterInfo : public M6502GenRegisterInfo {
public:
  M6502RegisterInfo();

  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;

  void eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  unsigned getFrameRegister(const MachineFunction &MF) const override;
};

} // end namespace llvm

#endif
