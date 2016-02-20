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
  addRegisterClass(MVT::i8, &M6502::GeneralRegClass);
  addRegisterClass(MVT::i16, &M6502::PtrRegClass);

  computeRegisterProperties(Subtarget.getRegisterInfo());

  //setOperationAction(ISD::LOAD, MVT::i16, Custom);
  //setOperationAction(ISD::STORE, MVT::i16, Custom);
  //setOperationAction(ISD::TRUNCATE, MVT::i8, Custom);
  //setTruncStoreAction(MVT::i16, MVT::i8, Expand);
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
      unsigned VReg = MF.addLiveIn(VA.getLocReg(),
                                   getRegClassFor(VA.getLocVT()));
      InVals.push_back(DAG.getCopyFromReg(Chain, dl, VReg, VA.getLocVT()));
    } else if (VA.isMemLoc()) {
      unsigned ValSize = VA.getValVT().getSizeInBits() / 8;
      int FI = MF.getFrameInfo()->CreateFixedObject(ValSize, VA.getLocMemOffset(), true);
      SDValue FIPtr = DAG.getFrameIndex(FI, getPointerTy(MF.getDataLayout()));
      SDValue Val = DAG.getLoad(VA.getLocVT(), dl, Chain, FIPtr,
                                MachinePointerInfo(), false, false, false, 0);
      InVals.push_back(Val);
    } else {
      llvm_unreachable("Argument must be located in register or stack");
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
  if (Outs.size() == 0)
    return true;
  if (Outs.size() > 1)
    return false;

  assert(Outs.size() == 1);

  if (Outs[0].VT == MVT::i8)
    return true;

  return false;
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
    // NOTE: If return value won't fit in registers, CanLowerReturn should
    // return false. LLVM will handle returning values on the stack.
    assert(VA.isRegLoc() && "Can only lower return into registers");

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
  case ISD::LOAD: return LowerLOAD(Op, DAG);
  case ISD::STORE: return LowerSTORE(Op, DAG);
  case ISD::TRUNCATE: return LowerTRUNCATE(Op, DAG);
  }

  return SDValue();
}

SDValue
M6502TargetLowering::LowerLOAD(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  LoadSDNode *Load = cast<LoadSDNode>(Op);

  assert(Load->getMemoryVT() == MVT::i16 &&
         "Custom LOAD lowering only implemented for i16");

  SDValue Ptr = Load->getBasePtr();

  SDValue LoVal = DAG.getLoad(MVT::i8, DL, Load->getChain(),
                              Load->getBasePtr(),
                              Load->getPointerInfo(), Load->isVolatile(),
                              Load->isNonTemporal(), Load->isInvariant(),
                              Load->getAlignment(), Load->getAAInfo(),
                              Load->getRanges());
  SDValue HiVal = DAG.getLoad(MVT::i8, DL, Load->getChain(),
                              DAG.getMemBasePlusOffset(Load->getBasePtr(),
                                                       1, DL),
                              Load->getPointerInfo(), Load->isVolatile(),
                              Load->isNonTemporal(), Load->isInvariant(),
                              Load->getAlignment(), Load->getAAInfo(),
                              Load->getRanges());

  // FIXME: are loval and hival backwards?
  SDValue Ops[] = {
    // Value
    DAG.getNode(ISD::BUILD_PAIR, DL, MVT::i16, HiVal, LoVal),
    // Chain
    DAG.getNode(ISD::TokenFactor, DL, MVT::Other, HiVal.getValue(1), LoVal.getValue(1))
  };
  return DAG.getMergeValues(Ops, DL);
}

SDValue
M6502TargetLowering::LowerSTORE(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);
  StoreSDNode *Store = cast<StoreSDNode>(Op);

  assert(Store->getMemoryVT() == MVT::i16 &&
         "Custom STORE lowering only implemented for i16");

  SDValue Val = Store->getValue();

  SDValue LoVal = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i8, Val,
                              DAG.getIntPtrConstant(0, DL));
  SDValue HiVal = DAG.getNode(ISD::EXTRACT_ELEMENT, DL, MVT::i8, Val,
                              DAG.getIntPtrConstant(1, DL));

  SDValue LoChain = DAG.getStore(Store->getChain(), DL, LoVal,
                                 Store->getBasePtr(),
                                 Store->getPointerInfo(), Store->isVolatile(),
                                 Store->isNonTemporal(), Store->getAlignment(),
                                 Store->getAAInfo());
  SDValue HiChain = DAG.getStore(Store->getChain(), DL, HiVal,
                                 DAG.getMemBasePlusOffset(Store->getBasePtr(),
                                                          1, DL),
                                 Store->getPointerInfo(), Store->isVolatile(),
                                 Store->isNonTemporal(), Store->getAlignment(),
                                 Store->getAAInfo());

  return DAG.getNode(ISD::TokenFactor, DL, MVT::Other, LoChain, HiChain);
}

SDValue
M6502TargetLowering::LowerTRUNCATE(SDValue Op, SelectionDAG &DAG) const {
  SDLoc DL(Op);

  errs() << "Asked to lower "; Op.dump();

  SDValue TruncMe = Op.getOperand(0);
  if (TruncMe.getValueType() == MVT::i16) {
    // TODO
    errs() << "TODO: implement i16 truncate lowering\n";
    errs() << "Lowering TRUNCATE of: "; TruncMe.dump();
    if (TruncMe.getOpcode() == ISD::BUILD_PAIR) {
      // FIXME: pull operand 0 or 1?
      // i8 = trunc (i16 = build-pair hi, lo)
      //    => lo
      return TruncMe.getOperand(1);
    } else if (TruncMe.getOpcode() == ISD::SRL
               && TruncMe.getOperand(0).getOpcode() == ISD::BUILD_PAIR
               && TruncMe.getOperand(1).getOpcode() == ISD::Constant
               && TruncMe.getConstantOperandVal(1) == 8) {
      // i8 = trunc (i16 = srl (i16 = build-pair hi, lo), 8)
      //    => hi
      SDValue Pair = TruncMe.getOperand(0);
      // FIXME: pull operand 0 or 1?
      return Pair.getOperand(0);
    }
  }
  
  return SDValue();
}

SDValue M6502TargetLowering::PerformDAGCombine(SDNode *N,
                                               DAGCombinerInfo &DCI) const {
  SelectionDAG &DAG = DCI.DAG;
  switch (N->getOpcode()) {
  default: break;
    // TODO: implement custom DAG combining
  }

  return SDValue();
}
