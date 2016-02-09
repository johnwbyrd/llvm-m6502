// TODO: header stuff

#include "M6502FrameLowering.h"
#include "M6502.h"
#include "M6502InstrInfo.h"
#include "M6502Subtarget.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;

M6502FrameLowering::M6502FrameLowering(const M6502Subtarget &STI)
  : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 1, 0) {}
  // FIXME: Is 0 for TransientStackAlignment valid? XCore does it.

void M6502FrameLowering::emitPrologue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  // TODO
}

void M6502FrameLowering::emitEpilogue(MachineFunction &MF,
                                      MachineBasicBlock &MBB) const {
  // TODO
}

bool M6502FrameLowering::hasFP(const MachineFunction &MF) const {
  // TODO (note: 6502 has very few registers)
  return false;
}
