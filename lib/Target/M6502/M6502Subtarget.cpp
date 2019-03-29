// TODO: header stuff

#include "M6502Subtarget.h"

using namespace llvm;

#define GET_SUBTARGETINFO_CTOR
#include "M6502GenSubtargetInfo.inc"

M6502Subtarget::M6502Subtarget(const Triple &TT, const std::string &CPU,
                               const std::string &FS,
                               const TargetMachine &TM)
    : M6502GenSubtargetInfo(TT, CPU, FS), FrameLowering(*this),
      TLInfo(TM, *this) {}
