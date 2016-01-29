// TODO: header stuff

#include "Mos6502ISelLowering.h"
#include "Mos6502RegisterInfo.h"
#include "Mos6502Subtarget.h"

using namespace llvm;

Mos6502TargetLowering::Mos6502TargetLowering(const TargetMachine &TM,
                                             const Mos6502Subtarget &Subtarget)
    : TargetLowering(TM) {

  // FIXME: It is not clear what is required here

  addRegisterClass(MVT::i8, &Mos6502::ARegsRegClass);

  computeRegisterProperties(Subtarget.getRegisterInfo());
}

SDValue
Mos6502TargetLowering::LowerFormalArguments(SDValue Chain,
                                            CallingConv::ID CallConv,
                                            bool isVarArg,
                                      const SmallVectorImpl<ISD::InputArg> &Ins,
                                            SDLoc dl,
                                            SelectionDAG &DAG,
                                            SmallVectorImpl<SDValue> &InVals)
                                              const {
  // TODO
  return Chain;
}

SDValue
Mos6502TargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                   bool isVarArg,
                                   const SmallVectorImpl<ISD::OutputArg> &Outs,
                                   const SmallVectorImpl<SDValue> &OutVals,
                                   SDLoc dl, SelectionDAG &DAG) const {
  // TODO
  return Chain;
}
