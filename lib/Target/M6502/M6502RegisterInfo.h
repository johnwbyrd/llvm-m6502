// TODO: header stuff

#ifndef LLVM_LIB_TARGET_M6502_M6502REGISTERINFO_H
#define LLVM_LIB_TARGET_M6502_M6502REGISTERINFO_H

#define GET_REGINFO_HEADER
#include "M6502GenRegisterInfo.inc"

namespace llvm {

class M6502RegisterInfo : public M6502GenRegisterInfo {
public:
  M6502RegisterInfo();
  
  /// Return a null-terminated list of all of the callee-saved registers on
  /// this target. The register should be in the order of desired callee-save
  /// stack frame offset. The first register is closest to the incoming stack
  /// pointer if stack grows down, and vice versa.
  ///
  virtual const MCPhysReg*
  getCalleeSavedRegs(const MachineFunction *MF) const override;
  
  /// Returns a bitset indexed by physical register number indicating if a
  /// register is a special register that has particular uses and should be
  /// considered unavailable at all times, e.g. SP, RA. This is
  /// used by register scavenger to determine what registers are free.
  virtual BitVector getReservedRegs(const MachineFunction &MF) const override;

  
  /// Returns the largest super class of RC that is legal to use in the current
  /// sub-target and has the same spill size.
  /// The returned register class can be used to create virtual registers which
  /// means that all its registers can be copied and spilled.
  virtual const TargetRegisterClass *
  getLargestLegalSuperClass(const TargetRegisterClass *RC,
                            const MachineFunction &) const override;

  void eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  unsigned getFrameRegister(const MachineFunction &MF) const override;
};

} // end namespace llvm

#endif
