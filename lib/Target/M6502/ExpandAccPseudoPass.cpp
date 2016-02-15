// TODO: header stuff

// This file contains a pre-reg allocation pass to expand pseudo instructions
// that operate on the Acc register, such as ADDreg_pseudo, SUBreg_pseudo, etc.

#include "M6502.h"
#include "M6502InstrInfo.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
using namespace llvm;

namespace {

class ExpandAccPseudoPass : public MachineFunctionPass {
public:
  static char ID;
  ExpandAccPseudoPass() : MachineFunctionPass(ID) {}

  bool runOnMachineFunction(MachineFunction &MF) override;

  const char *getPassName() const override {
    return "M6502 Accumulator pseudo-instruction expansion pass";
  }

private:
  bool runOnMachineInstr(MachineBasicBlock &MBB, MachineInstr *MI);
  bool runOnBasicBlock(MachineBasicBlock &MBB);
};

char ExpandAccPseudoPass::ID = 0;

} // End of namespace

static unsigned ConvertRegPseudoToStackLoading(unsigned Pseudo) {
  switch (Pseudo) {
  default:
    llvm_unreachable(false && "Opcode has no stack-loading conversion");
    break;
  case M6502::ADDreg_pseudo:
    return M6502::ADDabs; // TODO: ADDstack
  case M6502::SUBreg_pseudo:
    return M6502::SUBabs; // TODO: SUBstack
  }
}

bool ExpandAccPseudoPass::runOnMachineInstr(MachineBasicBlock &MBB,
                                            MachineInstr *MI) {
  MachineFunction &MF = *MBB.getParent();
  const MachineRegisterInfo &MRI = MF.getRegInfo();
  const TargetInstrInfo *TII = MF.getSubtarget().getInstrInfo();
  const TargetRegisterInfo *TRI = MF.getSubtarget().getRegisterInfo();
  unsigned OldOpcode = MI->getOpcode();

  if (OldOpcode == M6502::ADDreg_pseudo
      || OldOpcode == M6502::SUBreg_pseudo) {
    // Spill operand 2 to stack
    MachineOperand &SpillMe = MI->getOperand(2);
    const TargetRegisterClass *SpillRC = MRI.getRegClass(SpillMe.getReg());

    // FIXME: is this the right way to spill to stack in a pre-RA pass?
    int StackSlot = MF.getFrameInfo()->CreateSpillStackObject(
        SpillRC->getSize(), SpillRC->getAlignment());
    TII->storeRegToStackSlot(MBB, MI, SpillMe.getReg(), false, StackSlot,
                             SpillRC, MF.getSubtarget().getRegisterInfo());

    // Replace pseudo-instruction with an instruction that loads from the
    // stack.
    // TODO: build some kind of ADDstack instruction.
    // FIXME: could/should we exploit the commutative property of ADDs here?
    unsigned NewOpcode = ConvertRegPseudoToStackLoading(OldOpcode);
    BuildMI(MBB, MI, MI->getDebugLoc(), TII->get(NewOpcode))
      .addOperand(MI->getOperand(0))
      .addOperand(MI->getOperand(1))
      .addImm(StackSlot);

    MI->eraseFromParent();
    return true;
  }

  return false;
}

bool ExpandAccPseudoPass::runOnBasicBlock(MachineBasicBlock &MBB) {
  bool Modified = false;

  // XXX: iterate through instructions in reverse to avoid breakage when MBB
  // is modified.
  MachineBasicBlock::reverse_iterator MII = MBB.rbegin(), E = MBB.rend();
  while (MII != E) {
    bool MIModified = runOnMachineInstr(MBB, &*MII);
    if (MIModified) {
      E = MBB.rend(); // End may have changed
      Modified = true;
      continue;
    }
    ++MII;
  }

  return Modified;
}

bool ExpandAccPseudoPass::runOnMachineFunction(MachineFunction &MF) {
  bool Modified = false;
  for (MachineBasicBlock &MBB : MF) {
    Modified |= runOnBasicBlock(MBB);
  }

  return Modified;
}

FunctionPass *llvm::createExpandAccPseudoPass() {
  return new ExpandAccPseudoPass();
}