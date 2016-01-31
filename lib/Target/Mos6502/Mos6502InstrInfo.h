// TODO: header stuff

#ifndef LLVM_LIB_TARGET_MOS6502_MOS6502INSTRINFO_H
#define LLVM_LIB_TARGET_MOS6502_MOS6502INSTRINFO_H

#include "Mos6502RegisterInfo.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "Mos6502GenInstrInfo.inc"

namespace llvm {

class Mos6502InstrInfo : public Mos6502GenInstrInfo {
  const Mos6502RegisterInfo RI;

public:
  const TargetRegisterInfo &getRegisterInfo() const { return RI; }

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                   DebugLoc DL, unsigned DestReg, unsigned SrcReg,
                   bool killSrc) const override;
};

} // end namespace llvm

#endif
