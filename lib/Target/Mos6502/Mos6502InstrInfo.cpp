// TODO: header stuff

#include "Mos6502InstrInfo.h"
#include "MCTargetDesc/Mos6502MCTargetDesc.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "Mos6502GenInstrInfo.inc"

void Mos6502InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator I,
                                   DebugLoc DL, unsigned DestReg,
                                   unsigned SrcReg,
                                   bool killSrc) const {
  if (SrcReg == Mos6502::A && DestReg == Mos6502::X) {
    BuildMI(MBB, I, DL, get(Mos6502::TAX));
  } else if (SrcReg == Mos6502::A && DestReg == Mos6502::Y) {
    BuildMI(MBB, I, DL, get(Mos6502::TAY));
  } else if (SrcReg == Mos6502::X && DestReg == Mos6502::A) {
    BuildMI(MBB, I, DL, get(Mos6502::TXA));
  } else if (SrcReg == Mos6502::Y && DestReg == Mos6502::A) {
    BuildMI(MBB, I, DL, get(Mos6502::TYA));
  } else {
    // FIXME: handle X->Y and Y->X?
    llvm_unreachable("Impossible reg-to-reg copy");
  }
}
