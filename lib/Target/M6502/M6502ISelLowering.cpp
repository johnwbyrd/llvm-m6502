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

  addRegisterClass(MVT::i8, &M6502::GeneralRegClass);
  addRegisterClass(MVT::i16, &M6502::PtrRegClass);

  computeRegisterProperties(Subtarget.getRegisterInfo());

  setOperationAction(ISD::GlobalAddress, MVT::i16, Custom);
  // TODO: ExternalSymbol, BlockAddress
  // TODO: It would be better to Expand these operations. Find some way to make
  // it work.
  setOperationAction(ISD::AND, MVT::i16, Custom);
  setOperationAction(ISD::ADD, MVT::i16, Custom);
  setOperationAction(ISD::SUB, MVT::i16, Custom);
  setOperationAction(ISD::ZERO_EXTEND, MVT::i16, Custom);
  setOperationAction(ISD::BR_CC, MVT::i8, Custom);

  setLoadExtAction(ISD::SEXTLOAD, MVT::i16, MVT::i8, Expand);
  setLoadExtAction(ISD::ZEXTLOAD, MVT::i16, MVT::i8, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::i16, MVT::i8, Expand);
}

const char *
M6502TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (static_cast<M6502ISD::NodeType>(Opcode)) {
    // TODO: Use .def to automate this like WebAssembly
  case M6502ISD::FIRST_NUMBER:
    break;
  case M6502ISD::WRAPPER:
    return "M6502ISD::WRAPPER";
  case M6502ISD::CALL:
    return "M6502ISD::CALL";
  case M6502ISD::RETURN:
    return "M6502ISD::RETURN";
  case M6502ISD::CMP:
    return "M6502ISD::CMP";
  case M6502ISD::BSET:
    return "M6502ISD::BSET";
  case M6502ISD::BCLEAR:
    return "M6502ISD::BCLEAR";
  case M6502ISD::PTRHI:
    return "M6502ISD::PTRHI";
  case M6502ISD::PTRLO:
    return "M6502ISD::PTRLO";
  case M6502ISD::BUILDPTR:
    return "M6502ISD::BUILDPTR";
  }
  return nullptr;
}

SDValue
M6502TargetLowering::LowerFormalArguments(
      SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
      const SmallVectorImpl<ISD::InputArg> & Ins, const SDLoc & dl,
      SelectionDAG & DAG, SmallVectorImpl<SDValue> & InVals) const {
  // TODO
  MachineFunction &MF = DAG.getMachineFunction();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, MF, ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeFormalArguments(Ins, CC_M6502);

  for (unsigned i = 0; i < ArgLocs.size(); ++i) {
    CCValAssign &VA = ArgLocs[i];

    if (VA.isMemLoc()) {
      // FIXME: CreateFixedObject might be the wrong solution here. Do the research.
      int FI = MF.getFrameInfo()->CreateFixedObject(
        VA.getValVT().getStoreSize(), VA.getLocMemOffset(), true);
      SDValue FIPtr = DAG.getFrameIndex(FI, getPointerTy(MF.getDataLayout()));
      SDValue Val = DAG.getLoad(VA.getLocVT(), dl, Chain, FIPtr, MachinePointerInfo());
      InVals.push_back(Val);
    } else {
      llvm_unreachable("Argument must be located in memory");
    }
  }

  return Chain;
}

SDValue
M6502TargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                               SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG = CLI.DAG;
  MachineFunction &MF = DAG.getMachineFunction();
  SDLoc &dl = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins = CLI.Ins;
  SDValue Chain = CLI.Chain;
  SDValue Callee = CLI.Callee;
  bool &isTailCall = CLI.IsTailCall;
  CallingConv::ID CallConv = CLI.CallConv;
  bool isVarArg = CLI.IsVarArg;

  if (isVarArg) {
    // TODO: support varargs
    report_fatal_error("M6502 does not currently support varargs");
    return SDValue();
  }

  // M6502 target does not yet support tail call optimization. (TODO)
  isTailCall = false;

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCArgInfo(CallConv, isVarArg, MF, ArgLocs, *DAG.getContext());
  CCArgInfo.AnalyzeCallOperands(Outs, CC_M6502);

  // TODO: CALLSEQ_START instruction?

  SmallVector<SDValue, 12> MemOpChains;

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];

    SDValue Arg = OutVals[i];

    if (VA.isMemLoc()) {
      // FIXME: CreateFixedObject might be the wrong solution here. Do the research.
      int FI = MF.getFrameInfo()->CreateFixedObject(
        VA.getValVT().getStoreSize(), VA.getLocMemOffset(), true);
      SDValue FIPtr = DAG.getFrameIndex(FI, getPointerTy(MF.getDataLayout()));
      // TODO: special support for ByVals? please test.
      SDValue MemOp = DAG.getStore(Chain, dl, Arg, FIPtr, MachinePointerInfo());
      MemOpChains.push_back(MemOp);
    } else {
      llvm_unreachable("Invalid argument location");
    }
  }

  // Transform all store nodes into one single node because all stores are
  // independent of each other.
  if (!MemOpChains.empty()) {
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, MemOpChains);
  }

  // (Copied from MSP430ISelLowering.cpp)
  // If the callee is a GlobalAddress node (quite common, every direct call is)
  // turn it into a TargetGlobalAddress node so that legalize doesn't hack it.
  // Likewise ExternalSymbol -> TargetExternalSymbol.
  // FIXME: is this necessary?
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl, MVT::i16);
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), MVT::i16);

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain = DAG.getNode(M6502ISD::CALL, dl, NodeTys, Chain, Callee);
  // TODO: use glue?

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCReturnInfo(CallConv, isVarArg, MF, RVLocs, *DAG.getContext());
  CCReturnInfo.AnalyzeCallResult(Ins, CC_M6502);

  for (unsigned i = 0, e = RVLocs.size(); i != e; ++i) {
    CCValAssign &VA = RVLocs[i];

    if (VA.isMemLoc()) {
      // FIXME: CreateFixedObject might be the wrong solution here. Do the research.
      int FI = MF.getFrameInfo()->CreateFixedObject(
        VA.getValVT().getStoreSize(), VA.getLocMemOffset(), true);
      SDValue FIPtr = DAG.getFrameIndex(FI, getPointerTy(MF.getDataLayout()));
      // TODO: special support for ByVals? please test.
      SDValue Load = DAG.getLoad(VA.getLocVT(), dl, Chain, FIPtr, MachinePointerInfo());
      InVals.push_back(Load.getValue(0));
      Chain = Load.getValue(1);
    } else {
      llvm_unreachable("Return value must be in memory location");
    }
  }

  // TODO: CALLSEQ_END instruction?

  return Chain;
}

bool
M6502TargetLowering::CanLowerReturn(CallingConv::ID CallConv,
                                    MachineFunction &MF, bool isVarArg,
                                    const SmallVectorImpl<ISD::OutputArg> &Outs,
                                    LLVMContext &Context) const {
  // NOTE: M6502 lowers all return values to stack locations.
  return true;
}

SDValue
M6502TargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                 bool isVarArg,
                                 const SmallVectorImpl<ISD::OutputArg> & Outs,
                                 const SmallVectorImpl<SDValue> & OutVals,
                                 const SDLoc & dl,
                                 SelectionDAG & DAG) const {
  // TODO
  // XXX: RetOps stuff comes from WebAssemblyIselLowering and others
  MachineFunction &MF = DAG.getMachineFunction();

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, MF, RVLocs, *DAG.getContext());

  CCInfo.AnalyzeReturn(Outs, RetCC_M6502);

  SmallVector<SDValue, 4> RetOps(1, Chain);

  for (unsigned i = 0; i < RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];

    if (VA.isMemLoc()) {
      int FI = MF.getFrameInfo()->CreateFixedObject(
        VA.getValVT().getStoreSize(), VA.getLocMemOffset(), true);
      SDValue FIPtr = DAG.getFrameIndex(FI, getPointerTy(MF.getDataLayout()));
      SDValue RetOp = DAG.getStore(Chain, dl, OutVals[i], FIPtr, MachinePointerInfo());
      RetOps.push_back(RetOp);
    } else {
      llvm_unreachable("Return value must be located in memory");
    }
  }

  if (!RetOps.empty()) {
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, RetOps);
  }

  // Generate return instruction chained to output registers
  // FIXME: is glue needed?
  return DAG.getNode(M6502ISD::RETURN, dl, MVT::Other, Chain);
}

SDValue
M6502TargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::GlobalAddress: return LowerGlobalAddress(Op, DAG);
  case ISD::AND:
    return LowerAND(Op, DAG);
  case ISD::ADD:
  case ISD::SUB:
    return LowerADDSUB(Op, DAG);
  case ISD::ZERO_EXTEND:
    return LowerZERO_EXTEND(Op, DAG);
  case ISD::BR_CC: return LowerBR_CC(Op, DAG);
  default:
    llvm_unreachable("Custom lowering not implemented for operation");
    break;
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

SDValue M6502TargetLowering::LowerGlobalAddress(SDValue Op,
                                                SelectionDAG &DAG) const {
  // See MSP430ISelLowering.cpp
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  int64_t Offset = cast<GlobalAddressSDNode>(Op)->getOffset();
  auto PtrVT = getPointerTy(DAG.getDataLayout());

  // Create the TargetGlobalAddress node, folding in the constant offset.
  SDValue Result = DAG.getTargetGlobalAddress(GV, SDLoc(Op), PtrVT, Offset);
  return DAG.getNode(M6502ISD::WRAPPER, SDLoc(Op), PtrVT, Result);
}

SDValue M6502TargetLowering::LowerAND(SDValue Op, SelectionDAG &DAG) const {
  if (Op.getValueType() == MVT::i16) {
    // Based on DAGTypeLegalizer::ExpandIntRes_Logical in LegalizeIntegerTypes.cpp.
    // It would be better to find some way to call that code instead of copying
    // it here.
    SDValue LHS = Op.getOperand(0);
    SDValue RHS = Op.getOperand(1);
    SDLoc dl(Op);
    // Expand the subcomponents.
    SDValue LL, LH, RL, RH;
    LL = DAG.getNode(M6502ISD::PTRLO, dl, MVT::i8, LHS);
    LH = DAG.getNode(M6502ISD::PTRHI, dl, MVT::i8, LHS);
    RL = DAG.getNode(M6502ISD::PTRLO, dl, MVT::i8, RHS);
    RH = DAG.getNode(M6502ISD::PTRHI, dl, MVT::i8, RHS);

    SDValue Lo, Hi;
    Lo = DAG.getNode(Op.getOpcode(), dl, LL.getValueType(), LL, RL);
    Hi = DAG.getNode(Op.getOpcode(), dl, LL.getValueType(), LH, RH);
    return DAG.getNode(M6502ISD::BUILDPTR, dl, MVT::i16, Hi, Lo);
  } else {
    return SDValue();
  }
}

SDValue M6502TargetLowering::LowerADDSUB(SDValue Op, SelectionDAG &DAG) const {
  if (Op.getValueType() == MVT::i16) {
    // Manually expand 16-bit addition. Despite having 16-bit pointers, the CPU
    // cannot perform this operation natively.
    // TODO: there are some types of pointer addition that CAN be performed
    // natively by the CPU, for example, instructions that use absolute-indexed
    // addressing modes. Try to support this.
    // Based on DAGTypeLegalizer::ExpandIntRes_ADDSUB in LegalizeIntegerTypes.cpp.
    // It would be better to find some way to call that code instead of copying
    // it here.
    SDValue LHS = Op.getOperand(0);
    SDValue RHS = Op.getOperand(1);
    SDLoc dl(Op);
    // Expand the subcomponents.
    SDValue LHSL, LHSH, RHSL, RHSH;
    LHSL = DAG.getNode(M6502ISD::PTRLO, dl, MVT::i8, LHS);
    LHSH = DAG.getNode(M6502ISD::PTRHI, dl, MVT::i8, LHS);
    RHSL = DAG.getNode(M6502ISD::PTRLO, dl, MVT::i8, RHS);
    RHSH = DAG.getNode(M6502ISD::PTRHI, dl, MVT::i8, RHS);

    EVT NVT = LHSL.getValueType();
    SDValue LoOps[2] = { LHSL, RHSL };
    SDValue HiOps[3] = { LHSH, RHSH };

    SDVTList VTList = DAG.getVTList(NVT, MVT::Glue);
    SDValue Lo, Hi;
    if (Op.getOpcode() == ISD::ADD) {
      Lo = DAG.getNode(ISD::ADDC, dl, VTList, LoOps);
      HiOps[2] = Lo.getValue(1);
      Hi = DAG.getNode(ISD::ADDE, dl, VTList, HiOps);
    } else {
      Lo = DAG.getNode(ISD::SUBC, dl, VTList, LoOps);
      HiOps[2] = Lo.getValue(1);
      Hi = DAG.getNode(ISD::SUBE, dl, VTList, HiOps);
    }
    return DAG.getNode(M6502ISD::BUILDPTR, dl, MVT::i16, Hi, Lo);
  } else {
    return SDValue(); // Use standard lowering
  }
}

SDValue M6502TargetLowering::LowerZERO_EXTEND(SDValue Op, SelectionDAG &DAG) const {
  if (Op.getValueType() == MVT::i16 && Op.getOperand(0).getValueType() == MVT::i8) {
    SDLoc dl(Op);
    SDValue Lo, Hi;
    Lo = Op.getOperand(0);
    Hi = DAG.getConstant(0, dl, MVT::i8);
    return DAG.getNode(M6502ISD::BUILDPTR, dl, MVT::i16, Hi, Lo);
  } else {
    return SDValue();
  }
}

SDValue M6502TargetLowering::LowerBR_CC(SDValue Op, SelectionDAG &DAG) const {
  SDValue Chain = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS = Op.getOperand(2);
  SDValue RHS = Op.getOperand(3);
  SDValue Dest = Op.getOperand(4);
  SDLoc dl(Op);

  // TODO: clean up, verify correctness
  M6502ISD::NodeType NodeType1;
  unsigned int FlagReg1 = M6502::NoRegister;
  M6502ISD::NodeType NodeType2;
  unsigned int FlagReg2 = M6502::NoRegister;
  switch (CC) {
  case ISD::SETEQ:
    NodeType1 = M6502ISD::BSET;
    FlagReg1 = M6502::ZF;
    break;
  case ISD::SETNE:
    NodeType1 = M6502ISD::BCLEAR;
    FlagReg1 = M6502::ZF;
    break;
  case ISD::SETLT: // signed less-than
    NodeType1 = M6502ISD::BSET;
    FlagReg1 = M6502::NF;
    break;
  case ISD::SETLE: // signed less-than or equal
    NodeType1 = M6502ISD::BSET;
    FlagReg1 = M6502::NF;
    NodeType2 = M6502ISD::BSET;
    FlagReg2 = M6502::ZF;
    break;
  case ISD::SETGT: // signed greater-than
    NodeType1 = M6502ISD::BCLEAR;
    FlagReg1 = M6502::NF;
    break;
  case ISD::SETGE: // signed greater-than or equal
    NodeType1 = M6502ISD::BCLEAR;
    FlagReg1 = M6502::NF;
    NodeType2 = M6502ISD::BSET;
    FlagReg2 = M6502::ZF;
    break;
  case ISD::SETULT: // unsigned less-than
    NodeType1 = M6502ISD::BSET;
    FlagReg1 = M6502::CF;
    break;
  case ISD::SETULE: // unsigned less-than or equal
    NodeType1 = M6502ISD::BSET;
    FlagReg1 = M6502::CF;
    NodeType2 = M6502ISD::BSET;
    FlagReg2 = M6502::ZF;
    break;
  case ISD::SETUGT: // unsigned greater-than
    NodeType1 = M6502ISD::BCLEAR;
    FlagReg1 = M6502::CF;
    break;
  case ISD::SETUGE: // unsigned greater-than or equal
    NodeType1 = M6502ISD::BCLEAR;
    FlagReg1 = M6502::CF;
    NodeType2 = M6502ISD::BSET;
    FlagReg2 = M6502::ZF;
    break;
  default:
    llvm_unreachable("Invalid integer condition");
    break;
  }
  
  // TODO: avoid generating CMP instruction if possible, e.g. if
  // an earlier SUB instruction put the desired condition in ZFlag.
  Chain = DAG.getCopyToReg(Chain, dl, M6502::R0, LHS); // Load LHS to R0 (todo: allocate virtual register?)
  SDValue CmpGlue = DAG.getNode(M6502ISD::CMP, dl, MVT::Glue, DAG.getRegister(M6502::R0, MVT::i8), RHS);

  if (FlagReg2 != M6502::NoRegister) {
    // FIXME: glue correct?
    SDValue Branch1 = DAG.getNode(NodeType1, dl, MVT::Glue, Chain,
                                  DAG.getRegister(FlagReg1, MVT::i1), Dest, CmpGlue);
    SDValue Branch2 = DAG.getNode(NodeType2, dl, Op.getValueType(), Chain,
                                  DAG.getRegister(FlagReg2, MVT::i1), Dest, Branch1);
    return Branch2;
  } else {
    SDValue Branch1 = DAG.getNode(NodeType1, dl, Op.getValueType(), Chain,
                                  DAG.getRegister(FlagReg1, MVT::i1), Dest, CmpGlue);
    return Branch1;
  }
}
