// TODO: header stuff

#ifndef LLVM_LIB_TARGET_MOS6502_MOS6502ISELLOWERING_H
#define LLVM_LIB_TARGET_MOS6502_MOS6502ISELLOWERING_H

#include "llvm/Target/TargetLowering.h"

namespace llvm {
class Mos6502Subtarget;

class Mos6502TargetLowering : public TargetLowering {
public:
  Mos6502TargetLowering(const TargetMachine &TM,
                        const Mos6502Subtarget &Subtarget);

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               SDLoc dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals,
                      SDLoc dl, SelectionDAG &DAG) const override;
};

} // end namespace llvm

#endif
