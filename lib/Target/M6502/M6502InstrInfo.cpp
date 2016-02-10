// TODO: header stuff

#include "M6502InstrInfo.h"
#include "MCTargetDesc/M6502MCTargetDesc.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "M6502GenInstrInfo.inc"

void M6502InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator I,
                                 DebugLoc DL, unsigned DestReg,
                                 unsigned SrcReg,
                                 bool killSrc) const {
  if (M6502::AccRegClass.contains(SrcReg) &&
      M6502::IndexRegClass.contains(DestReg)) {
    // TAX, TAY
    BuildMI(MBB, I, DL, get(M6502::TAI), DestReg)
      .addReg(SrcReg, getKillRegState(killSrc));
  } else if (M6502::IndexRegClass.contains(SrcReg) &&
             M6502::AccRegClass.contains(DestReg)) {
    // TXA, TYA
    BuildMI(MBB, I, DL, get(M6502::TIA), DestReg)
      .addReg(SrcReg, getKillRegState(killSrc));
  } else {
    // FIXME: handle X->Y and Y->X?
    llvm_unreachable("Impossible reg-to-reg copy");
  }
}
