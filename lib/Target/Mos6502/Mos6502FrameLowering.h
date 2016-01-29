// TODO: header stuff

#ifndef LLVM_LIB_TARGET_MOS6502_MOS6502FRAMELOWERING_H
#define LLVM_LIB_TARGET_MOS6502_MOS6502FRAMELOWERING_H

#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
class Mos6502Subtarget;

class Mos6502FrameLowering : public TargetFrameLowering {
public:
  Mos6502FrameLowering(const Mos6502Subtarget &STI);

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  bool hasFP(const MachineFunction &MF) const override;
};

} // end namespace llvm

#endif
