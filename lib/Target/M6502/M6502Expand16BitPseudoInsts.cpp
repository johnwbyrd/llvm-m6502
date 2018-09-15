// TODO: header stuff

#include "M6502.h"
#include "M6502InstrInfo.h"
#include "M6502RegisterInfo.h"
#include "M6502TargetMachine.h"

#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

#define DEBUG_TYPE "m6502-expand-16bit-pseudos"

#define M6502_EXPAND_16BIT_PSEUDO_NAME "M6502 16-bit pseudo instruction expansion pass"

namespace {

class M6502Expand16BitPseudo : public MachineFunctionPass {
public:
  static char ID;

  M6502Expand16BitPseudo() : MachineFunctionPass(ID) {
    initializeM6502Expand16BitPseudoPass(*PassRegistry::getPassRegistry());
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

  StringRef getPassName() const override { return M6502_EXPAND_16BIT_PSEUDO_NAME; }

private:
  typedef MachineBasicBlock Block;
  typedef Block::iterator BlockIt;

  const M6502RegisterInfo *TRI;
  const TargetInstrInfo *TII;

  MachineInstrBuilder buildMI(Block &MBB, BlockIt MBBI, unsigned Opcode) {
    return BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(Opcode));
  }

  MachineInstrBuilder buildMI(Block &MBB, BlockIt MBBI, unsigned Opcode,
    unsigned DstReg) {
    return BuildMI(MBB, MBBI, MBBI->getDebugLoc(), TII->get(Opcode), DstReg);
  }

  bool expandMBB(Block &MBB);
  bool expandMI(Block &MBB, BlockIt MBBI);

  bool expandLogic(unsigned Op, Block &MBB, BlockIt MBBI);
};

char M6502Expand16BitPseudo::ID = 0;

// Credit: The code in this file is largely stolen from -- I mean, inspired by --
//         AVRExpandPseudoInsts.cpp.
bool M6502Expand16BitPseudo::runOnMachineFunction(MachineFunction &MF) {
  DEBUG(dbgs() << "Hello. 16-bit pseudo-instructions are expanded here.\n");

  bool Modified = false;

  const M6502Subtarget &STI = MF.getSubtarget<M6502Subtarget>();
  TRI = STI.getRegisterInfo();
  TII = STI.getInstrInfo();

  for (Block &MBB : MF) {
    bool ContinueExpanding = true;
    unsigned ExpandCount = 0;

    // Continue expanding the block until all pseudos are expanded.
    do {
      assert(ExpandCount < 10 && "pseudo expand limit reached");

      bool BlockModified = expandMBB(MBB);
      Modified |= BlockModified;
      ExpandCount++;

      ContinueExpanding = BlockModified;
    } while (ContinueExpanding);
  }

  return Modified;
}

bool M6502Expand16BitPseudo::expandMBB(MachineBasicBlock &MBB) {
  bool Modified = false;

  BlockIt MBBI = MBB.begin(), E = MBB.end();
  while (MBBI != E) {
    BlockIt NMBBI = std::next(MBBI);
    Modified |= expandMI(MBB, MBBI);
    MBBI = NMBBI;
  }

  return Modified;
}

bool M6502Expand16BitPseudo::expandMI(Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  int Opcode = MBBI->getOpcode();

  switch (Opcode) {
  case M6502::ORA_16:
    return expandLogic(Opcode, MBB, MBBI);
  }

  return false;
}

bool M6502Expand16BitPseudo::
expandLogic(unsigned Op, Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  DEBUG(dbgs() << "Expanding logic instruction: "; MI.dump(););
  unsigned SrcLoReg, SrcHiReg, DstLoReg, DstHiReg;
  unsigned DstReg = MI.getOperand(0).getReg();
  unsigned SrcReg = MI.getOperand(2).getReg();
  bool DstIsDead = MI.getOperand(0).isDead();
  bool DstIsKill = MI.getOperand(1).isKill();
  bool SrcIsKill = MI.getOperand(2).isKill();
  TRI->splitReg(SrcReg, SrcLoReg, SrcHiReg);
  TRI->splitReg(DstReg, DstLoReg, DstHiReg);

  if (Op != M6502::ORA_16) {
    llvm_unreachable("Unimplemented: expandLogic expected ORA_16");
  }

  Op = M6502::ORA_8;

  auto MIBLO = buildMI(MBB, MBBI, Op)
    .addReg(DstLoReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstLoReg, getKillRegState(DstIsKill))
    .addReg(SrcLoReg, getKillRegState(SrcIsKill));

  auto MIBHI = buildMI(MBB, MBBI, Op)
    .addReg(DstHiReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstHiReg, getKillRegState(DstIsKill))
    .addReg(SrcHiReg, getKillRegState(SrcIsKill));

  MI.eraseFromParent();
  return true;
}

} // end of anonymous namespace

INITIALIZE_PASS(M6502Expand16BitPseudo, "m6502-expand-16bit-pseudo",
                M6502_EXPAND_16BIT_PSEUDO_NAME, false, false)
  
namespace llvm {

FunctionPass *createM6502Expand16BitPseudoPass() { return new M6502Expand16BitPseudo(); }

} // end of namespace llvm
