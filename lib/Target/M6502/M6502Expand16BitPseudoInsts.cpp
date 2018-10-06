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
  bool expandArithImm(unsigned OpLo, unsigned OpHi, Block &MBB, BlockIt MBBI);
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
  case M6502::AD0_16:
  case M6502::ADD_16:
    return expandArith(M6502::AD0_8, M6502::ADC_8, MBB, MBBI);
  case M6502::AD0_imm_16:
  case M6502::ADD_imm_16:
    return expandArithImm(M6502::AD0_imm, M6502::ADC_imm, MBB, MBBI);
  case M6502::ADC_16:
    return expandArith(M6502::ADC_8, M6502::ADC_8, MBB, MBBI);
  case M6502::ADC_imm_16:
    return expandArithImm(M6502::ADC_imm, M6502::ADC_imm, MBB, MBBI);
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
  case M6502::SB1_16:
  case M6502::SUB_16:
    return expandArith(M6502::SB1_8, M6502::SBC_8, MBB, MBBI); // FIXME: are hi and lo ops in the correct order?
  case M6502::SB1_imm_16:
  case M6502::SUB_imm_16:
    return expandArithImm(M6502::SB1_imm, M6502::SBC_imm, MBB, MBBI);
  case M6502::SBC_16:
    return expandArith(M6502::SBC_8, M6502::SBC_8, MBB, MBBI);
  case M6502::SBC_imm_16:
    return expandArithImm(M6502::SBC_imm, M6502::SBC_imm, MBB, MBBI);
  }

  return false;
}

bool M6502Expand16BitPseudo::
expandArith(unsigned OpLo, unsigned OpHi, Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  DEBUG(dbgs() << "Expanding arithmetic instruction: "; MI.dump(););

  assert(MI.getOperand(0).getReg() == MI.getOperand(1).getReg() &&
         "Destination and source operands must be the same");
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

  auto MIBLO = buildMI(MBB, MBBI, OpLo)
    .addReg(DstLoReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstLoReg, getKillRegState(DstIsKill))
    .addReg(SrcLoReg, getKillRegState(SrcIsKill));

  auto MIBHI = buildMI(MBB, MBBI, OpHi)
    .addReg(DstHiReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstHiReg, getKillRegState(DstIsKill))
    .addReg(SrcHiReg, getKillRegState(SrcIsKill));

  if (ImpIsDead)
    MIBHI->getOperand(3).setIsDead();

  DEBUG(dbgs() << "Expanded to:\nMIBLO = " << *MIBLO << "\nMIBHI = " << *MIBHI << "\n");

  MI.eraseFromParent();
  return true;
}

static void expandImmOperand(const MachineOperand &MOp, MachineInstrBuilder &MIBLO, MachineInstrBuilder &MIBHI) {
  switch (MOp.getType()) {
  case MachineOperand::MO_GlobalAddress: {
    const GlobalValue *GV = MOp.getGlobal();
    int64_t Offs = MOp.getOffset();
    unsigned TF = MOp.getTargetFlags();
    MIBLO.addGlobalAddress(GV, Offs, TF | M6502II::MO_LO); // TODO: indicate lo/hi operands in printed assembly
    MIBHI.addGlobalAddress(GV, Offs, TF | M6502II::MO_HI);
    break;
  }
  case MachineOperand::MO_Immediate: {
    unsigned Imm = MOp.getImm();
    MIBLO.addImm(Imm & 0xff);
    MIBHI.addImm((Imm >> 8) & 0xff);
    break;
  }
  default:
    llvm_unreachable("Unknown immediate operand type!");
  }
}

bool M6502Expand16BitPseudo::
expandArithImm(unsigned OpLo, unsigned OpHi, Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  DEBUG(dbgs() << "Expanding arithmetic immediate instruction: "; MI.dump(););

  assert(MI.getOperand(0).getReg() == MI.getOperand(1).getReg() &&
         "Destination and source operands must be the same");
  unsigned DstLoReg, DstHiReg;
  unsigned DstReg = MI.getOperand(0).getReg();
  bool DstIsDead = MI.getOperand(0).isDead();
  bool SrcIsKill = MI.getOperand(1).isKill();
  bool ImpIsDead = MI.getOperand(3).isDead();
  TRI->splitReg(DstReg, DstLoReg, DstHiReg);

  // FIXME: ensure carry flag is handled correctly.

  auto MIBLO = buildMI(MBB, MBBI, OpLo)
    .addReg(DstLoReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstLoReg, getKillRegState(SrcIsKill));

  auto MIBHI = buildMI(MBB, MBBI, OpHi)
    .addReg(DstHiReg, RegState::Define | getDeadRegState(DstIsDead))
    .addReg(DstHiReg, getKillRegState(SrcIsKill));

  expandImmOperand(MI.getOperand(2), MIBLO, MIBHI);

  if (ImpIsDead)
    MIBHI->getOperand(3).setIsDead();

  MI.eraseFromParent();
  return true;
}

bool M6502Expand16BitPseudo::
expandLogic(unsigned NewOp, Block &MBB, BlockIt MBBI) {
  MachineInstr &MI = *MBBI;
  DEBUG(dbgs() << "Expanding logic instruction: "; MI.dump(););

  assert(MI.getOperand(0).getReg() == MI.getOperand(1).getReg() &&
         "Destination and source operands must be the same");
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

  assert(MI.getOperand(0).getReg() == MI.getOperand(1).getReg() &&
         "Destination and source operands must be the same");
  unsigned DstLoReg, DstHiReg;
  unsigned DstReg = MI.getOperand(0).getReg();
  bool DstIsDead = MI.getOperand(0).isDead();
  bool SrcIsKill = MI.getOperand(1).isKill();
  TRI->splitReg(DstReg, DstLoReg, DstHiReg);

  auto MIBLO = buildMI(MBB, MBBI, Op)
      .addReg(DstLoReg, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(DstLoReg, getKillRegState(SrcIsKill));

  auto MIBHI = buildMI(MBB, MBBI, Op)
      .addReg(DstHiReg, RegState::Define | getDeadRegState(DstIsDead))
      .addReg(DstHiReg, getKillRegState(SrcIsKill));

  expandImmOperand(MI.getOperand(2), MIBLO, MIBHI);

  // TODO: filter out redundant logic instructions (i.e. AND x, 0xff; OR x, 0)

  MI.eraseFromParent();
  return true;
}

} // end of anonymous namespace

INITIALIZE_PASS(M6502Expand16BitPseudo, "m6502-expand-16bit-pseudo",
                M6502_EXPAND_16BIT_PSEUDO_NAME, false, false)
  
namespace llvm {

FunctionPass *createM6502Expand16BitPseudoPass() { return new M6502Expand16BitPseudo(); }

} // end of namespace llvm
