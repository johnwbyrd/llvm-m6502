// TODO: header stuff

#include "M6502ISelLowering.h"
#include "M6502RegisterInfo.h"
#include "M6502Subtarget.h"
#include "MCTargetDesc/M6502MCTargetDesc.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"

using namespace llvm;

#define DEBUG_TYPE "m6502-lower"

#include "M6502GenCallingConv.inc"

M6502TargetLowering::M6502TargetLowering(const TargetMachine &TM,
                                         const M6502Subtarget &Subtarget)
    : TargetLowering(TM) {

  addRegisterClass(MVT::i8, &M6502::GeneralRegClass);
  // XXX: this is a hack to get conditional branches to work. This might break
  // code that uses i1 variables. Find a better solution.
  addRegisterClass(MVT::i1, &M6502::FlagRegClass);

  computeRegisterProperties(Subtarget.getRegisterInfo());

  setOperationAction(ISD::GlobalAddress, MVT::i16, Custom);
  setOperationAction(ISD::FrameIndex, MVT::i16, Custom);
  setOperationAction(ISD::ExternalSymbol, MVT::i16, Custom);
  setOperationAction(ISD::MUL, MVT::i8, LibCall);
  setOperationAction(ISD::BR_CC, MVT::i8, Custom);

  // TODO: support pre-indexed loads and stores
  //setIndexedLoadAction(ISD::PRE_INC, MVT::i8, Legal);
  //setIndexedStoreAction(ISD::PRE_INC, MVT::i8, Legal);
}

MVT M6502TargetLowering::getScalarShiftAmountTy(const DataLayout &DL,
                                                EVT LHSTy) const {
  // NOTE: Without this override, LLVM assumes that a shift amount
  // is the size of a pointer. However, i16's are illegal in M6502.
  return MVT::i8;
}

const char *
M6502TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (static_cast<M6502ISD::NodeType>(Opcode)) {
    // TODO: Use .def to automate this like WebAssembly
  case M6502ISD::FIRST_NUMBER:
    break;
  case M6502ISD::ABSADDR:
    return "M6502ISD::ABSADDR";
  case M6502ISD::HILOADDR:
    return "M6502ISD::HILOADDR";
  case M6502ISD::FIADDR:
    return "M6502ISD::FIADDR";
  case M6502ISD::ADDRHI:
    return "M6502ISD::ADDRHI";
  case M6502ISD::ADDRLO:
    return "M6502ISD::ADDRLO";
  case M6502ISD::FIHI:
    return "M6502ISD::FIHI";
  case M6502ISD::FILO:
    return "M6502ISD::FILO";
  case M6502ISD::LOADFROM:
    return "M6502ISD::LOADFROM";
  case M6502ISD::STORETO:
    return "M6502ISD::STORETO";
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

// Convert i16 pointer to M6502 address operand for LOADFROM and STORETO nodes.
static SDValue ConvertPtrToAddress(const SDValue &Ptr, const SDLoc &dl,
                                   SelectionDAG &DAG) {
  assert(Ptr.getValueType() == MVT::i16);

  // Attempt to recombine an address pair to a frameindex or globaladdress,
  // possibly with one or more added constant offsets.
  SDValue Walker = Ptr;
  int64_t Offset = 0;
  // Walk down pointer nodes, accumulating offsets.
  while (Walker.getOpcode() == ISD::ADD) {
    SDValue LHS = Walker.getOperand(0);
    SDValue RHS = Walker.getOperand(1);
    if (isa<ConstantSDNode>(LHS)) {
      Offset += cast<ConstantSDNode>(LHS)->getSExtValue();
      Walker = RHS;
    } else if (isa<ConstantSDNode>(RHS)) {
      Offset += cast<ConstantSDNode>(RHS)->getSExtValue();
      Walker = LHS;
    } else {
      // Pointer added to non-constant. Do not attempt to recombine.
      Walker = SDValue();
      break;
    }
  }

  if (Walker && Walker.getOpcode() == ISD::BUILD_PAIR) {
    SDValue Lo = Walker.getOperand(0);
    SDValue Hi = Walker.getOperand(1);
    if (Lo.getOpcode() == M6502ISD::ADDRLO &&
        Hi.getOpcode() == M6502ISD::ADDRHI &&
        Lo.getOperand(0) == Hi.getOperand(0)) {
      // Recombine absolute address
      SDValue Address = Lo.getOperand(0);
      assert(Address.getOpcode() == ISD::TargetGlobalAddress); // TODO: support texternalsym, etc.
      assert(Address.getValueType() == MVT::i16);
      // Add Offset if necessary
      if (Offset != 0) {
        GlobalAddressSDNode *GA = cast<GlobalAddressSDNode>(Address);
        Address = DAG.getTargetGlobalAddress(GA->getGlobal(), dl, MVT::i16,
                                             GA->getOffset() + Offset,
                                             GA->getTargetFlags());
      }
      return DAG.getNode(M6502ISD::ABSADDR, dl, MVT::Other, Address);
    } else if (Lo.getOpcode() == M6502ISD::FILO &&
               Hi.getOpcode() == M6502ISD::FIHI
               && Lo.getOperand(0) == Hi.getOperand(0)) {
      // Recombine frameindex
      SDValue Index = Lo.getOperand(0);
      assert(Index.getOpcode() == ISD::TargetConstant); // See LowerFrameIndex
      return DAG.getNode(M6502ISD::FIADDR, dl, MVT::Other,
                          Index, DAG.getTargetConstant(Offset, dl, MVT::i16));
    }
  }

  // Generic
  SDValue PtrLo = DAG.getNode(ISD::EXTRACT_ELEMENT, dl, MVT::i8, Ptr,
                              DAG.getTargetConstant(0, dl, MVT::i8));
  SDValue PtrHi = DAG.getNode(ISD::EXTRACT_ELEMENT, dl, MVT::i8, Ptr,
                              DAG.getTargetConstant(1, dl, MVT::i8));
  return DAG.getNode(M6502ISD::HILOADDR, dl, MVT::Other, PtrHi, PtrLo);
}

void M6502TargetLowering::LegalizeOperationTypes(SDNode *N,
                                             SmallVectorImpl<SDValue> &Results,
                                             SelectionDAG &DAG) const {
  switch (N->getOpcode()) {
  default:
    break;
  case ISD::LOAD:
  {
    LoadSDNode *Load = cast<LoadSDNode>(N);
    if (Load->getValueType(0) != MVT::i8) { // TODO: use getMemoryVT? is there a difference?
      return; // Allow legalizer to handle non-i8 loads
    }
    SDValue BasePtr = Load->getBasePtr(); // TODO: handle indexing and truncating here?
    if (BasePtr->getValueType(0) != MVT::i16) {
      return; // Allow legalizer to handle when pointer is not type i16... (FIXME: is this ok?)
    }

    DEBUG(dbgs() << "Load BasePtr: "; BasePtr->dumprFull(&DAG));

    SDLoc dl(N);
    SDValue Address = ConvertPtrToAddress(BasePtr, dl, DAG);
    SDValue Result = DAG.getNode(M6502ISD::LOADFROM, dl,
                                 DAG.getVTList(MVT::i8, MVT::Other),
                                 Load->getChain(), Address);
    Results.push_back(Result.getValue(0)); // Value
    Results.push_back(Result.getValue(1)); // Chain
    break;
  }
  case ISD::STORE:
  {
    StoreSDNode *Store = cast<StoreSDNode>(N);
    if (Store->getMemoryVT() != MVT::i8) {
      return; // Allow legalizer to handle non-i8 stores
    }
    SDValue BasePtr = Store->getBasePtr(); // TODO: handle indexing and truncating here?
    if (BasePtr->getValueType(0) != MVT::i16) {
      return; // Allow legalizer to handle when pointer is not type i16... (FIXME: is this ok?)
    }

    DEBUG(dbgs() << "Store BasePtr: "; BasePtr->dumprFull(&DAG));

    SDLoc dl(N);
    SDValue Value = Store->getValue();
    SDValue Address = ConvertPtrToAddress(BasePtr, dl, DAG);
    SDValue Result = DAG.getNode(M6502ISD::STORETO, dl, MVT::Other,
                                 Store->getChain(), Value, Address);
    Results.push_back(Result);
    break;
  }
  }
}

SDValue
M6502TargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::GlobalAddress: return LowerGlobalAddress(Op, DAG);
  case ISD::FrameIndex: return LowerFrameIndex(Op, DAG);
  case ISD::ExternalSymbol: return LowerExternalSymbol(Op, DAG);
  case ISD::BR_CC: return LowerBR_CC(Op, DAG);
  default:
    llvm_unreachable("Custom lowering not implemented for operation");
    break;
  }

  return SDValue();
}

void M6502TargetLowering::ReplaceNodeResults(SDNode *N,
                                             SmallVectorImpl<SDValue> &Results,
                                             SelectionDAG &DAG) const {
  SDLoc dl(N);
  switch (N->getOpcode()) {
  case ISD::GlobalAddress:
    // FIXME: this work is repeated from LowerOperation?
    Results.push_back(LowerGlobalAddress(SDValue(N, 0), DAG));
    break;
  case ISD::FrameIndex:
    // FIXME: this work is repeated from LowerOperation?
    Results.push_back(LowerFrameIndex(SDValue(N, 0), DAG));
    break;
  default:
    llvm_unreachable("Do not know how to custom type legalize this operation!");
    break;
  }
}

SDValue M6502TargetLowering::PerformDAGCombine(SDNode *N,
                                               DAGCombinerInfo &DCI) const {
  SelectionDAG &DAG = DCI.DAG;
  switch (N->getOpcode()) {
  default:
    break;
  }

  return SDValue();
}

SDValue M6502TargetLowering::LowerGlobalAddress(SDValue Op,
                                                SelectionDAG &DAG) const {
  // Transform GlobalAddress to prevent LLVM from trying to legalize the i16.
  SDLoc dl(Op);
  const GlobalAddressSDNode *GA = cast<GlobalAddressSDNode>(Op);
  SDValue Address = DAG.getTargetGlobalAddress(GA->getGlobal(), dl, MVT::i16,
                                               GA->getOffset(),
                                               GA->getTargetFlags());
  SDValue Hi = DAG.getNode(M6502ISD::ADDRHI, dl, MVT::i8, Address);
  SDValue Lo = DAG.getNode(M6502ISD::ADDRLO, dl, MVT::i8, Address);
  // NOTE: The order of operands for BUILD_PAIR is Lo, Hi.
  return DAG.getNode(ISD::BUILD_PAIR, dl, MVT::i16, Lo, Hi);
}

SDValue M6502TargetLowering::LowerFrameIndex(SDValue Op,
                                             SelectionDAG &DAG) const {
  // Transform FrameIndex to prevent LLVM from trying to legalize the i16.
  SDLoc dl(Op);
  const FrameIndexSDNode *FI = cast<FrameIndexSDNode>(Op);
  assert(FI->getValueType(0) == MVT::i16);
  SDValue Index = DAG.getTargetConstant(FI->getIndex(), dl, MVT::i16);
  SDValue Hi = DAG.getNode(M6502ISD::FIHI, dl, MVT::i8, Index);
  SDValue Lo = DAG.getNode(M6502ISD::FILO, dl, MVT::i8, Index);
  // NOTE: The order of operands for BUILD_PAIR is Lo, Hi.
  return DAG.getNode(ISD::BUILD_PAIR, dl, MVT::i16, Lo, Hi);
}

SDValue M6502TargetLowering::LowerExternalSymbol(SDValue Op,
                                                 SelectionDAG &DAG) const {
  // Transform GlobalAddress to prevent LLVM from trying to legalize the i16.
  SDLoc dl(Op);
  const ExternalSymbolSDNode *ES = cast<ExternalSymbolSDNode>(Op);
  SDValue Address = DAG.getTargetExternalSymbol(ES->getSymbol(), MVT::i16,
                                                ES->getTargetFlags());
  SDValue Hi = DAG.getNode(M6502ISD::ADDRHI, dl, MVT::i8, Address);
  SDValue Lo = DAG.getNode(M6502ISD::ADDRLO, dl, MVT::i8, Address);
  // NOTE: The order of operands for BUILD_PAIR is Lo, Hi.
  return DAG.getNode(ISD::BUILD_PAIR, dl, MVT::i16, Lo, Hi);
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
