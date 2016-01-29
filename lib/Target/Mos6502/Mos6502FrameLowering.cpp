// TODO: header stuff

#include "Mos6502FrameLowering.h"

using namespace llvm;

Mos6502FrameLowering::Mos6502FrameLowering(const Mos6502Subtarget &STI)
  : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 1, 0) {}
  // FIXME: Is 0 for TransientStackAlignment valid? XCore does it.

void Mos6502FrameLowering::emitPrologue(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {
  // TODO
}

void Mos6502FrameLowering::emitEpilogue(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {
  // TODO
}

bool Mos6502FrameLowering::hasFP(const MachineFunction &MF) const {
  // TODO (note: 6502 has very few registers)
  return false;
}
