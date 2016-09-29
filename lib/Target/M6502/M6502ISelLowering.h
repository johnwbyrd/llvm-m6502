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
  ABSADDR, // absolute address (timm, tglobaladdr, texternalsym, tblockaddr)
  HILOADDR, // address formed of Hi, Lo bytes
  FIADDR, // frame index address
  ADDRHI, // hi element of global address
  ADDRLO, // lo element of global address
  FIHI, // hi element of frame index address
  FILO, // lo element of frame index address
  LOADFROM,
  STORETO,
  CALL,
  RETURN,
  CMP,
  BSET,
  BCLEAR,
  SELECT_CC,
  NFLAG, ZFLAG, CFLAG, VFLAG,
};

} // end namespace M6502ISD

class M6502TargetLowering : public TargetLowering {
public:
  M6502TargetLowering(const TargetMachine &TM,
                      const M6502Subtarget &Subtarget);

  MVT getScalarShiftAmountTy(const DataLayout &DL, EVT LHSTy) const override;
  EVT getSetCCResultType(const DataLayout &DL, LLVMContext &Context,
                         EVT VT) const override;

  const char *getTargetNodeName(unsigned Opcode) const override;

  SDValue LowerFormalArguments(
      SDValue /*Chain*/, CallingConv::ID /*CallConv*/, bool /*isVarArg*/,
      const SmallVectorImpl<ISD::InputArg> & /*Ins*/, const SDLoc & /*dl*/,
      SelectionDAG & /*DAG*/, SmallVectorImpl<SDValue> & /*InVals*/) const override;
  
  SDValue LowerCall(CallLoweringInfo &/*CLI*/,
                    SmallVectorImpl<SDValue> &/*InVals*/) const override;

  bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF,
                      bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      LLVMContext &Context) const override;

  SDValue LowerReturn(SDValue /*Chain*/, CallingConv::ID /*CallConv*/,
                      bool /*isVarArg*/,
                      const SmallVectorImpl<ISD::OutputArg> & /*Outs*/,
                      const SmallVectorImpl<SDValue> & /*OutVals*/,
                      const SDLoc & /*dl*/,
                      SelectionDAG & /*DAG*/) const override;

  // Provide custom lowering hooks for some operations.
  void LegalizeOperationTypes(SDNode *N,
                              SmallVectorImpl<SDValue> &Results,
                              SelectionDAG &DAG) const override;
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;
  void ReplaceNodeResults(SDNode *N,
                          SmallVectorImpl<SDValue> &Results,
                          SelectionDAG &DAG) const override;

  SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const override;

  MachineBasicBlock *
  EmitInstrWithCustomInserter(MachineInstr &MI, MachineBasicBlock *MBB)
      const override;

private:
  SDValue LowerGlobalAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerFrameIndex(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerExternalSymbol(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
};

} // end namespace llvm

#endif
