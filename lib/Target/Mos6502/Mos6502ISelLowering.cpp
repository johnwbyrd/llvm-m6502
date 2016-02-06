// TODO: header stuff

#include "Mos6502ISelLowering.h"
#include "Mos6502RegisterInfo.h"
#include "Mos6502Subtarget.h"
#include "MCTargetDesc/Mos6502MCTargetDesc.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAG.h"

using namespace llvm;

#include "Mos6502GenCallingConv.inc"

Mos6502TargetLowering::Mos6502TargetLowering(const TargetMachine &TM,
                                             const Mos6502Subtarget &Subtarget)
    : TargetLowering(TM) {

  // FIXME: It is not clear what is required here

  addRegisterClass(MVT::i8, &Mos6502::AccRegClass);
  addRegisterClass(MVT::i8, &Mos6502::IndexRegClass);
  addRegisterClass(MVT::i8, &Mos6502::RegRegClass);
  addRegisterClass(MVT::i16, &Mos6502::PtrRegClass);

  computeRegisterProperties(Subtarget.getRegisterInfo());
}

const char *
Mos6502TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (static_cast<Mos6502ISD::NodeType>(Opcode)) {
    // TODO: Use .def to automate this like WebAssembly
  case Mos6502ISD::FIRST_NUMBER:
    break;
  case Mos6502ISD::RETURN:
    return "Mos6502ISD::RETURN";
  }
  return nullptr;
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
  MachineFunction &MF = DAG.getMachineFunction();

  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, MF, ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeFormalArguments(Ins, CC_Mos6502);

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
Mos6502TargetLowering::CanLowerReturn(CallingConv::ID CallConv,
                                      MachineFunction &MF, bool isVarArg,
                                    const SmallVectorImpl<ISD::OutputArg> &Outs,
                                      LLVMContext &Context) const {
  // TODO
  // Mos6502 can't currently handle returning tuples.
  return Outs.size() <= 1;
}

SDValue
Mos6502TargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                   bool isVarArg,
                                   const SmallVectorImpl<ISD::OutputArg> &Outs,
                                   const SmallVectorImpl<SDValue> &OutVals,
                                   SDLoc dl, SelectionDAG &DAG) const {
  // TODO
  // XXX: RetOps stuff comes from WEbAssemblyIselLowering and others
  MachineFunction &MF = DAG.getMachineFunction();

  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, MF, RVLocs, *DAG.getContext());

  CCInfo.AnalyzeReturn(Outs, RetCC_Mos6502);

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
  return DAG.getNode(Mos6502ISD::RETURN, dl, MVT::Other, RetOps);
}
