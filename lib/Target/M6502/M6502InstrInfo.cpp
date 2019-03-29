// TODO: header stuff

#include "M6502InstrInfo.h"
#include "MCTargetDesc/M6502MCTargetDesc.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

#define DEBUG_TYPE "m6502-instr-info"

#define GET_INSTRINFO_CTOR_DTOR
#include "M6502GenInstrInfo.inc"

M6502InstrInfo::M6502InstrInfo()
  : M6502GenInstrInfo(M6502::ADJCALLSTACKDOWN, M6502::ADJCALLSTACKUP) {}

void M6502InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 const DebugLoc &DL,
                                 unsigned DestReg, unsigned SrcReg,
                                 bool KillSrc) const {
  if (M6502::GPR8RegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MI, DL, get(M6502::T_reg), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
  } else if (M6502::DREGSRegClass.contains(DestReg, SrcReg)) {
    BuildMI(MBB, MI, DL, get(M6502::T_reg_16), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
  } else {
    llvm_unreachable("Registers could not be copied");
  }
}

/// isLoadFromStackSlot - If the specified machine instruction is a direct
/// load from a stack slot, return the virtual or physical register number of
/// the destination along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than loading from the stack slot.
unsigned M6502InstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                             int &FrameIndex) const {
  // TODO
  // FIXME: All load instructions affect N and Z flags.
  DEBUG(dbgs() << "Is Load from Stack Slot?: "; MI.dump());
  if (MI.getOpcode() == M6502::LD_stack || MI.getOpcode() == M6502::LD_stack_16) {
    const MachineOperand &Dest = MI.getOperand(0);
    const MachineOperand &FI = MI.getOperand(1);
    const MachineOperand &Offset = MI.getOperand(2);
    // FIXME: can offset be non-zero?
    if (Dest.isReg() && FI.isFI() && Offset.isImm() && Offset.getImm() == 0) {
      FrameIndex = FI.getIndex();
      return Dest.getReg();
    }
  }

  return 0;
}

/// isStoreToStackSlot - If the specified machine instruction is a direct
/// store to a stack slot, return the virtual or physical register number of
/// the source reg along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than storing to the stack slot.
unsigned M6502InstrInfo::isStoreToStackSlot(const MachineInstr &MI,
                                            int &FrameIndex) const {
  // TODO
  DEBUG(dbgs() << "Is Store to Stack Slot?: "; MI.dump());
  if (MI.getOpcode() == M6502::ST_stack || MI.getOpcode() == M6502::ST_stack_16) {
    const MachineOperand &Src = MI.getOperand(0);
    const MachineOperand &FI = MI.getOperand(1);
    const MachineOperand &Offset = MI.getOperand(2);
    // FIXME: can offset be non-zero?
    if (Src.isReg() && FI.isFI() && Offset.isImm() && Offset.getImm() == 0) {
      FrameIndex = FI.getIndex();
      return Src.getReg();
    }
  }

  return 0;
}

void M6502InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator MI,
                                         unsigned SrcReg, bool isKill,
                                         int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (MI != MBB.end()) {
    DL = MI->getDebugLoc();
  }

  // TODO: Use a MachineMemOperand.

  if (M6502::GPR8RegClass.hasSubClassEq(RC)) {
    BuildMI(MBB, MI, DL, get(M6502::ST_stack))
      .addReg(SrcReg, getKillRegState(isKill))
      .addFrameIndex(FrameIndex)
      .addImm(0);
  } else if (M6502::DREGSRegClass.hasSubClassEq(RC)) {
    BuildMI(MBB, MI, DL, get(M6502::ST_stack_16))
      .addReg(SrcReg, getKillRegState(isKill))
      .addFrameIndex(FrameIndex)
      .addImm(0);
  } else {
    llvm_unreachable("Register class could not be stored to stack");
  }
}


void M6502InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator MI,
                                          unsigned DestReg, int FrameIndex,
                                          const TargetRegisterClass *RC,
                                          const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (MI != MBB.end()) {
    DL = MI->getDebugLoc();
  }

  // TODO: Use a MachineMemOperand.

  if (M6502::GPR8RegClass.hasSubClassEq(RC)) {
    BuildMI(MBB, MI, DL, get(M6502::LD_stack), DestReg)
      .addFrameIndex(FrameIndex)
      .addImm(0);
  } else if (M6502::DREGSRegClass.hasSubClassEq(RC)) {
    BuildMI(MBB, MI, DL, get(M6502::LD_stack_16), DestReg)
      .addFrameIndex(FrameIndex)
      .addImm(0);
  } else {
    llvm_unreachable("Register class could not be loaded from stack");
  }
}

/// foldMemoryOperand - If this target supports it, fold a load or store of
/// the specified stack slot into the specified machine instruction for the
/// specified operand(s).  If this is possible, the target should perform the
/// folding and return true, otherwise it should return false.  If it folds
/// the instruction, it is likely that the MachineInstruction the iterator
/// references has been changed.
MachineInstr *M6502InstrInfo::foldMemoryOperandImpl(
    MachineFunction &MF, MachineInstr &MI,
    ArrayRef<unsigned> Ops,
    MachineBasicBlock::iterator InsertPt,
    int FrameIndex,
    LiveIntervals *LIS) const {
  // TODO: fold if possible
  DEBUG(dbgs() << "Asked to fold stack operand: "; MI.dump());
  return nullptr;
}

/// foldMemoryOperand - Same as the previous version except it allows folding
/// of any load and store from / to any address, not just from a specific
/// stack slot.
MachineInstr *M6502InstrInfo::foldMemoryOperandImpl(
    MachineFunction &MF, MachineInstr &MI, ArrayRef<unsigned> Ops,
    MachineBasicBlock::iterator InsertPt, MachineInstr &LoadMI,
    LiveIntervals *LIS) const {
  // TODO: fold if possible
  DEBUG(dbgs() << "Asked to fold address operand: "; MI.dump());
  return nullptr;
}
