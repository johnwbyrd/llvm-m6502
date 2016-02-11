// TODO: header stuff

#include "M6502ISelLowering.h"
#include "M6502RegisterInfo.h"
#include "M6502Subtarget.h"
#include "MCTargetDesc/M6502MCTargetDesc.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"

using namespace llvm;

#include "M6502GenCallingConv.inc"

M6502TargetLowering::M6502TargetLowering(const TargetMachine &TM,
                                         const M6502Subtarget &Subtarget)
    : TargetLowering(TM) {

  // FIXME: It is not clear what is required here

  addRegisterClass(MVT::i8, &M6502::AccRegClass);
  addRegisterClass(MVT::i8, &M6502::IndexRegClass);
  addRegisterClass(MVT::i8, &M6502::RegRegClass);
  addRegisterClass(MVT::i16, &M6502::PtrRegClass);

  computeRegisterProperties(Subtarget.getRegisterInfo());

  setOperationAction(ISD::ADD, MVT::i8, Custom);
}

const char *
M6502TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (static_cast<M6502ISD::NodeType>(Opcode)) {
    // TODO: Use .def to automate this like WebAssembly
  case M6502ISD::FIRST_NUMBER:
    break;
  case M6502ISD::RETURN:
    return "M6502ISD::RETURN";
  }
  return nullptr;
}

SDValue
M6502TargetLowering::LowerFormalArguments(SDValue Chain,
                                          CallingConv::ID CallConv,
                                          bool isVarArg,
                                      const SmallVectorImpl<ISD::InputArg> &Ins,
                                          SDLoc dl,
                                          SelectionDAG &DAG,
                                          SmallVectorImpl<SDValue> &InVals)
                                            const {
  // TODO
  MachineFunction &MF = DAG.getMachineFunction();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, MF, ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeFormalArguments(Ins, CC_M6502);

  for (unsigned i = 0; i < ArgLocs.size(); ++i) {
    CCValAssign &VA = ArgLocs[i];

    if (VA.isRegLoc()) {
      unsigned VReg = MF.addLiveIn(VA.getLocReg(), getRegClassFor(VA.getLocVT()));
      InVals.push_back(DAG.getCopyFromReg(Chain, dl, VReg, VA.getLocVT()));
    } else {
      // TODO
      llvm_unreachable("Stack-allocated arguments are not handled");
    }
  }

  return Chain;
}

bool
M6502TargetLowering::CanLowerReturn(CallingConv::ID CallConv,
                                    MachineFunction &MF, bool isVarArg,
                                    const SmallVectorImpl<ISD::OutputArg> &Outs,
                                    LLVMContext &Context) const {
  // TODO
  // M6502 can't currently handle returning tuples.
  return Outs.size() <= 1;
}

SDValue
M6502TargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                 bool isVarArg,
                                 const SmallVectorImpl<ISD::OutputArg> &Outs,
                                 const SmallVectorImpl<SDValue> &OutVals,
                                 SDLoc dl, SelectionDAG &DAG) const {
  // TODO
  // XXX: RetOps stuff comes from WebAssemblyIselLowering and others
  MachineFunction &MF = DAG.getMachineFunction();

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, MF, RVLocs, *DAG.getContext());

  CCInfo.AnalyzeReturn(Outs, RetCC_M6502);

  // FIXME: Is Glue necessary?
  SDValue Glue;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  for (unsigned i = 0; i < RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers");

    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), OutVals[i], Glue);
    Glue = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain; // Update chain.

  if (Glue) {
    RetOps.push_back(Glue);
  }

  // Generate return instruction chained to output registers
  return DAG.getNode(M6502ISD::RETURN, dl, MVT::Other, RetOps);
}

SDValue
M6502TargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  default:
    llvm_unreachable("Custom lowering not implemented for operation");
    break;
  case ISD::ADD:
    return LowerADD(Op, DAG);
  }
}

SDValue
M6502TargetLowering::LowerADD(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);

  SDValue OperA = Op.getOperand(0);
  SDValue OperB = Op.getOperand(1);

  if (OperA.getOpcode() == ISD::CopyFromReg &&
      OperB.getOpcode() == ISD::CopyFromReg) {
    // 6502 cannot add reg to reg. One operand must come from memory.
    // Due to LLVM limitations, we must custom-lower ADD to store one operand
    // to memory.
    // XXX: Our method is as follows:
    //      Store operand B to an arbitrary address (60) and then load it again.
    //      Set isVolatile to prevent LLVM from recombining the Store and Load
    //      nodes into another CopyFromReg.
    //      Then, LLVM will match the DAG pattern with the ADDabs instruction:
    //      (set A, (add A, (load B)))
    // TODO: Store to a reasonable location like the stack; choose which operand
    // to store more smaerter; don't use volatiles
    SDValue Ptr = DAG.getConstant(60, DL, MVT::i8);
    OperB = DAG.getStore(OperB.getValue(1), DL, OperB, Ptr, MachinePointerInfo(),
      true, false, 0);
    OperB = DAG.getLoad(MVT::i8, DL, OperB, Ptr, MachinePointerInfo(), true, false, false, 0);
    return DAG.getNode(ISD::ADD, DL, MVT::i8, OperA, OperB);
  } else {
    // Use default lowering
    return Op;
  }
}
