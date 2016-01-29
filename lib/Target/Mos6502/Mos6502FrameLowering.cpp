// TODO: header stuff

#include "Mos6502FrameLowering.h"
#include "Mos6502.h"
#include "Mos6502InstrInfo.h"
#include "Mos6502Subtarget.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

Mos6502FrameLowering::Mos6502FrameLowering(const Mos6502Subtarget &STI)
  : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 1, 0) {}
  // FIXME: Is 0 for TransientStackAlignment valid? XCore does it.

void Mos6502FrameLowering::emitPrologue(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {
  // XXX: generate an instruction for testing
  MachineBasicBlock::iterator MBBI = MBB.begin();
  const Mos6502InstrInfo &TII = *MF.getSubtarget<Mos6502Subtarget>().getInstrInfo();
  DebugLoc dl; // FIXME: what is this?
  BuildMI(MBB, MBBI, dl, TII.get(Mos6502::NOT), Mos6502::A);
}

void Mos6502FrameLowering::emitEpilogue(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {
  // TODO
}

bool Mos6502FrameLowering::hasFP(const MachineFunction &MF) const {
  // TODO (note: 6502 has very few registers)
  return false;
}
