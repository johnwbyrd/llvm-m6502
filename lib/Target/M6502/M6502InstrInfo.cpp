// TODO: header stuff

#include "M6502InstrInfo.h"
#include "MCTargetDesc/M6502MCTargetDesc.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "M6502GenInstrInfo.inc"

void M6502InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 const DebugLoc &DL,
                                 unsigned DestReg, unsigned SrcReg,
                                 bool KillSrc) const {
  BuildMI(MBB, MI, DL, get(M6502::T_reg), DestReg)
    .addReg(SrcReg, getKillRegState(KillSrc));
}

/// isLoadFromStackSlot - If the specified machine instruction is a direct
/// load from a stack slot, return the virtual or physical register number of
/// the destination along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than loading from the stack slot.
unsigned M6502InstrInfo::isLoadFromStackSlot(const MachineInstr &MI,
                                             int &FrameIndex) const {
  // TODO
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
  return 0;
}

void M6502InstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator MBBI,
                                         unsigned SrcReg, bool isKill,
                                         int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         const TargetRegisterInfo *TRI) const {
  // TODO: test
  DebugLoc DL = MBBI->getDebugLoc();
  if (M6502::GeneralRegClass.hasSubClassEq(RC)) {
    BuildMI(MBB, MBBI, DL, get(M6502::ST_stack))
      .addReg(SrcReg, getKillRegState(isKill))
      .addFrameIndex(FrameIndex)
      .addImm(0);
  } else {
    llvm_unreachable("Register class could not be stored to stack");
  }
}


void M6502InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator MBBI,
                                          unsigned DestReg, int FrameIndex,
                                          const TargetRegisterClass *RC,
                                          const TargetRegisterInfo *TRI) const {
  // TODO: test
  DebugLoc DL = MBBI->getDebugLoc();
  if (M6502::GeneralRegClass.hasSubClassEq(RC)) {
    BuildMI(MBB, MBBI, DL, get(M6502::LD_stack), DestReg)
      .addFrameIndex(FrameIndex)
      .addImm(0);
  } else {
    llvm_unreachable("Register class could not be loaded from stack");
  }
}

MachineInstr *M6502InstrInfo::foldMemoryOperandImpl(
    MachineFunction &MF, MachineInstr &MI,
    ArrayRef<unsigned> Ops,
    MachineBasicBlock::iterator InsertPt,
    int FrameIndex,
    LiveIntervals *LIS) const {
  // TODO: fold if possible
  return nullptr;
}

MachineInstr *M6502InstrInfo::foldMemoryOperandImpl(
    MachineFunction &MF, MachineInstr &MI, ArrayRef<unsigned> Ops,
    MachineBasicBlock::iterator InsertPt, MachineInstr &LoadMI,
    LiveIntervals *LIS) const {
  // TODO: fold if possible
  return nullptr;
}
