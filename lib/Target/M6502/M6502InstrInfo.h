// TODO: header stuff

#ifndef LLVM_LIB_TARGET_M6502_M6502INSTRINFO_H
#define LLVM_LIB_TARGET_M6502_M6502INSTRINFO_H

#include "M6502RegisterInfo.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "M6502GenInstrInfo.inc"

namespace llvm {

class M6502InstrInfo : public M6502GenInstrInfo {
  const M6502RegisterInfo RI;

public:
  const TargetRegisterInfo &getRegisterInfo() const { return RI; }

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                   DebugLoc DL, unsigned DestReg, unsigned SrcReg,
                   bool killSrc) const override;
};

} // end namespace llvm

#endif
