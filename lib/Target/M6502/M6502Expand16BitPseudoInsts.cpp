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

  bool expandArith(unsigned OpLo, unsigned OpHi, Block &MBB, BlockIt MBBI);
  bool expandLogic(unsigned NewOp, Block &MBB, BlockIt MBBI);
  bool expandLogicImm(unsigned NewOp, Block &MBB, BlockIt MBBI);
  bool isLogicImmOpRedundant(unsigned Op, unsigned ImmVal) const;
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
  case M6502::ADD_16:
    return expandArith(M6502::AD0_8, M6502::ADC_8, MBB, MBBI);
  case M6502::AND_16:
    return expandLogic(M6502::AND_8, MBB, MBBI);
  case M6502::AND_imm_16:
    return expandLogicImm(M6502::AND_imm, MBB, MBBI);
  case M6502::EOR_16:
    return expandLogic(M6502::EOR_8, MBB, MBBI);
  case M6502::EOR_imm_16:
    return expandLogicImm(M6502::EOR_imm, MBB, MBBI);
  case M6502::ORA_16:
    return expandLogic(M6502::ORA_8, MBB, MBBI);
  case M6502::ORA_imm_16:
    return expandLogicImm(M6502::ORA_imm, MBB, MBBI);
  case M6502::SUB_16:
    return expandArith(M6502::SB1_8, M6502::SBC_8, MBB, MBBI); // FIXME: are hi and lo ops in the correct order?
  }

  return false;
}

bool M6502Expand16BitPseudo::
expandArith(unsigned OpLo, unsigned OpHi, Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  DEBUG(dbgs() << "Expanding arithmetic instruction: "; MI.dump(););
  unsigned SrcLoReg, SrcHiReg, DstLoReg, DstHiReg;
  unsigned DstReg = MI.getOperand(0).getReg();
  unsigned SrcReg = MI.getOperand(2).getReg();
  bool DstIsDead = MI.getOperand(0).isDead();
  bool DstIsKill = MI.getOperand(1).isKill();
  bool SrcIsKill = MI.getOperand(2).isKill();
  bool ImpIsDead = MI.getOperand(3).isDead();
  TRI->splitReg(SrcReg, SrcLoReg, SrcHiReg);
  TRI->splitReg(DstReg, DstLoReg, DstHiReg);

  // FIXME: ensure carry flag is handled correctly.

  buildMI(MBB, MBBI, OpLo)
    .addReg(DstLoReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstLoReg, getKillRegState(DstIsKill))
    .addReg(SrcLoReg, getKillRegState(SrcIsKill));

  auto MIBHI = buildMI(MBB, MBBI, OpHi)
    .addReg(DstHiReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstHiReg, getKillRegState(DstIsKill))
    .addReg(SrcHiReg, getKillRegState(SrcIsKill));

  if (ImpIsDead)
    MIBHI->getOperand(3).setIsDead();

  MI.eraseFromParent();
  return true;
}

bool M6502Expand16BitPseudo::
expandLogic(unsigned NewOp, Block &MBB, BlockIt MBBI) {
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

  auto MIBLO = buildMI(MBB, MBBI, NewOp)
    .addReg(DstLoReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstLoReg, getKillRegState(DstIsKill))
    .addReg(SrcLoReg, getKillRegState(SrcIsKill));

  auto MIBHI = buildMI(MBB, MBBI, NewOp)
    .addReg(DstHiReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstHiReg, getKillRegState(DstIsKill))
    .addReg(SrcHiReg, getKillRegState(SrcIsKill));

  MI.eraseFromParent();
  return true;
}

bool M6502Expand16BitPseudo::
expandLogicImm(unsigned Op, Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  DEBUG(dbgs() << "Expanding logic immediate instruction: "; MI.dump(););
  unsigned DstLoReg, DstHiReg;
  unsigned DstReg = MI.getOperand(0).getReg();
  bool DstIsDead = MI.getOperand(0).isDead();
  bool SrcIsKill = MI.getOperand(1).isKill();
  unsigned Imm = MI.getOperand(2).getImm();
  unsigned Lo8 = Imm & 0xff;
  unsigned Hi8 = (Imm >> 8) & 0xff;
  TRI->splitReg(DstReg, DstLoReg, DstHiReg);

  if (!isLogicImmOpRedundant(Op, Lo8)) {
    auto MIBLO = buildMI(MBB, MBBI, Op)
      .addReg(DstLoReg, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(DstLoReg, getKillRegState(SrcIsKill))
      .addImm(Lo8);
  }

  if (!isLogicImmOpRedundant(Op, Hi8)) {
    auto MIBHI = buildMI(MBB, MBBI, Op)
      .addReg(DstHiReg, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(DstHiReg, getKillRegState(SrcIsKill))
      .addImm(Hi8);
  }

  MI.eraseFromParent();
  return true;
}

bool M6502Expand16BitPseudo::
isLogicImmOpRedundant(unsigned Op, unsigned ImmVal) const {

  // x AND 0xff is redundant.
  if (Op == M6502::AND_8 && ImmVal == 0xff)
    return true;

  // x OR 0 is redundant.
  if (Op == M6502::ORA_8 && ImmVal == 0x0)
    return true;

  return false;
}

} // end of anonymous namespace

INITIALIZE_PASS(M6502Expand16BitPseudo, "m6502-expand-16bit-pseudo",
                M6502_EXPAND_16BIT_PSEUDO_NAME, false, false)
  
namespace llvm {

FunctionPass *createM6502Expand16BitPseudoPass() { return new M6502Expand16BitPseudo(); }

} // end of namespace llvm
