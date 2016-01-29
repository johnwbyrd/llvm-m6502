// TODO: header stuff

#ifndef LLVM_LIB_TARGET_MOS6502_MOS6502SUBTARGET_H
#define LLVM_LIB_TARGET_MOS6502_MOS6502SUBTARGET_H

#include "Mos6502FrameLowering.h"
#include "Mos6502InstrInfo.h"
#include "Mos6502SelectionDAGInfo.h"
#include "Mos6502ISelLowering.h"
#include "llvm/Target/TargetSubtargetInfo.h"

#define GET_SUBTARGETINFO_HEADER
#include "Mos6502GenSubtargetInfo.inc"

namespace llvm {

class Mos6502Subtarget : public Mos6502GenSubtargetInfo {
  Mos6502InstrInfo InstrInfo;
  Mos6502FrameLowering FrameLowering;
  Mos6502TargetLowering TLInfo;
  Mos6502SelectionDAGInfo TSInfo;

public:
  Mos6502Subtarget(const Triple &TT, const std::string &CPU,
                   const std::string &FS, const TargetMachine &TM);

  const Mos6502InstrInfo *getInstrInfo() const override {
    return &InstrInfo;
  }
  const Mos6502FrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const Mos6502TargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }
  const Mos6502SelectionDAGInfo *getSelectionDAGInfo() const override {
    return &TSInfo;
  }
  const TargetRegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }
};

} // end namespace llvm

#endif
