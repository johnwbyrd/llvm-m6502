// TODO: header stuff

#ifndef LLVM_LIB_TARGET_M6502_M6502ISELLOWERING_H
#define LLVM_LIB_TARGET_M6502_M6502ISELLOWERING_H

#include "llvm/Target/TargetLowering.h"

namespace llvm {
class M6502Subtarget;

// M6502 custom SelectionDAG nodes
namespace M6502ISD {

enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  RETURN,
};

} // end namespace M6502ISD

class M6502TargetLowering : public TargetLowering {
public:
  M6502TargetLowering(const TargetMachine &TM,
                      const M6502Subtarget &Subtarget);

  const char *getTargetNodeName(unsigned Opcode) const override;

  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               SDLoc dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;

  bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF,
                      bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      LLVMContext &Context) const override;

  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals,
                      SDLoc dl, SelectionDAG &DAG) const override;


  // Provide custom lowering hooks for some operation.
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  SDValue LowerADD(SDValue Op, SelectionDAG &DAG) const;

  SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const override;
};

} // end namespace llvm

#endif
