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
  if (SrcReg == M6502::A && DestReg == M6502::X) {
    BuildMI(MBB, I, DL, get(M6502::TAI)).addReg(M6502::A).addReg(M6502::X);
  } else if (SrcReg == M6502::A && DestReg == M6502::Y) {
    BuildMI(MBB, I, DL, get(M6502::TAI)).addReg(M6502::A).addReg(M6502::Y);
  } else if (SrcReg == M6502::X && DestReg == M6502::A) {
    BuildMI(MBB, I, DL, get(M6502::TIA)).addReg(M6502::X).addReg(M6502::A);
  } else if (SrcReg == M6502::Y && DestReg == M6502::A) {
    BuildMI(MBB, I, DL, get(M6502::TIA)).addReg(M6502::Y).addReg(M6502::A);
  } else {
    // FIXME: handle X->Y and Y->X?
    llvm_unreachable("Impossible reg-to-reg copy");
  }
}
