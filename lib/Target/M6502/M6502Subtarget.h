// TODO: header stuff

#ifndef LLVM_LIB_TARGET_M6502_M6502SUBTARGET_H
#define LLVM_LIB_TARGET_M6502_M6502SUBTARGET_H

#include "M6502FrameLowering.h"
#include "M6502InstrInfo.h"
#include "M6502ISelLowering.h"
#include "llvm/Target/TargetSubtargetInfo.h"

#define GET_SUBTARGETINFO_HEADER
#include "M6502GenSubtargetInfo.inc"

namespace llvm {

class M6502Subtarget : public M6502GenSubtargetInfo {
  M6502InstrInfo InstrInfo;
  M6502FrameLowering FrameLowering;
  M6502TargetLowering TLInfo;

public:
  M6502Subtarget(const Triple &TT, const std::string &CPU,
                 const std::string &FS, const TargetMachine &TM);

  const M6502InstrInfo *getInstrInfo() const override {
    return &InstrInfo;
  }
  const M6502FrameLowering *getFrameLowering() const override {
    return &FrameLowering;
  }
  const M6502TargetLowering *getTargetLowering() const override {
    return &TLInfo;
  }
  const TargetRegisterInfo *getRegisterInfo() const override {
    return &InstrInfo.getRegisterInfo();
  }
};

} // end namespace llvm

#endif
