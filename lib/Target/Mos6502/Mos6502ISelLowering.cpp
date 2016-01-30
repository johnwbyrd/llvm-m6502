// TODO: header stuff

#include "Mos6502ISelLowering.h"
#include "Mos6502RegisterInfo.h"
#include "Mos6502Subtarget.h"
#include "MCTargetDesc/Mos6502MCTargetDesc.h"
#include "llvm/CodeGen/SelectionDAG.h"

using namespace llvm;

Mos6502TargetLowering::Mos6502TargetLowering(const TargetMachine &TM,
                                             const Mos6502Subtarget &Subtarget)
    : TargetLowering(TM) {

  // FIXME: It is not clear what is required here

  addRegisterClass(MVT::i8, &Mos6502::ARegsRegClass);

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
  SmallVector<SDValue, 4> RetOps(1, Chain);

  if (Outs.size() == 1) {
    // FIXME: How do we know it's OutVals[0]?
    // This should generate instructions to copy OutVals[0] to register A.
    Chain = DAG.getCopyToReg(Chain, dl, Mos6502::A, OutVals[0]);
    RetOps.push_back(DAG.getRegister(Mos6502::A, OutVals[0].getValueType()));
  } else if (Outs.size() == 0) {
	  // Do nothing
  } else {
    // TODO
    assert(false && "Mos6502 doesn't support more than 1 return value");
  }

  RetOps[0] = Chain; // Update chain.

  // Generate return instruction chained to output registers
  return DAG.getNode(Mos6502ISD::RETURN, dl, MVT::Other, RetOps);
}
