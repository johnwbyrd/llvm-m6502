// TODO: header stuff

#ifndef LLVM_LIB_TARGET_M6502_M6502FRAMELOWERING_H
#define LLVM_LIB_TARGET_M6502_M6502FRAMELOWERING_H

#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
class M6502Subtarget;

class M6502FrameLowering : public TargetFrameLowering {
public:
  M6502FrameLowering(const M6502Subtarget &STI);

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  bool hasFP(const MachineFunction &MF) const override;
};

} // end namespace llvm

#endif
