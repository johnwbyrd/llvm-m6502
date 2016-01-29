// TODO: header stuff

#include "Mos6502Subtarget.h"

using namespace llvm;

#define GET_SUBTARGETINFO_CTOR
#include "Mos6502GenSubtargetInfo.inc"

Mos6502Subtarget::Mos6502Subtarget(const Triple &TT, const std::string &CPU,
                                   const std::string &FS,
                                   const TargetMachine &TM)
    : Mos6502GenSubtargetInfo(TT, CPU, FS), FrameLowering(*this),
      TLInfo(TM, *this) {}
