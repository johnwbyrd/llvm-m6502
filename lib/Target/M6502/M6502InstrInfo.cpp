// TODO: header stuff

#include "M6502InstrInfo.h"
#include "MCTargetDesc/M6502MCTargetDesc.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"

using namespace llvm;

#define GET_INSTRINFO_CTOR_DTOR
#include "M6502GenInstrInfo.inc"

void M6502InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator I,
                                 DebugLoc DL, unsigned DestReg,
                                 unsigned SrcReg,
                                 bool killSrc) const {
  if (M6502::AccRegClass.contains(SrcReg) &&
      M6502::IndexRegClass.contains(DestReg)) {
    // TAX, TAY
    BuildMI(MBB, I, DL, get(M6502::TAI), DestReg)
      .addReg(SrcReg, getKillRegState(killSrc));
  } else if (M6502::IndexRegClass.contains(SrcReg) &&
             M6502::AccRegClass.contains(DestReg)) {
    // TXA, TYA
    BuildMI(MBB, I, DL, get(M6502::TIA), DestReg)
      .addReg(SrcReg, getKillRegState(killSrc));
  } else {
    // FIXME: handle X->Y and Y->X?
    llvm_unreachable("Impossible reg-to-reg copy");
  }
}

/// isLoadFromStackSlot - If the specified machine instruction is a direct
/// load from a stack slot, return the virtual or physical register number of
/// the destination along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than loading from the stack slot.
unsigned M6502InstrInfo::isLoadFromStackSlot(const MachineInstr *MI,
                                             int &FrameIndex) const {

  // TODO
  return 0;
}

/// isStoreToStackSlot - If the specified machine instruction is a direct
/// store to a stack slot, return the virtual or physical register number of
/// the source reg along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than storing to the stack slot.
unsigned M6502InstrInfo::isStoreToStackSlot(const MachineInstr *MI,
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

  // XXX: store stack vars to address equal to FrameIndex
  // TODO: Check for valid RC
  DebugLoc DL = MBBI->getDebugLoc();
  BuildMI(MBB, MBBI, DL, get(M6502::STRabs))
    .addReg(SrcReg, getKillRegState(isKill))
    .addImm(FrameIndex);
}


void M6502InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator MBBI,
                                          unsigned DestReg, int FrameIndex,
                                          const TargetRegisterClass *RC,
                                          const TargetRegisterInfo *TRI) const {
  
  // XXX: load stack vars from address equal to FrameIndex
  // TODO: Check for valid RC
  DebugLoc DL = MBBI->getDebugLoc();
  BuildMI(MBB, MBBI, DL, get(M6502::LDRabs), DestReg)
    .addImm(FrameIndex);
}

MachineInstr *M6502InstrInfo::foldMemoryOperandImpl(
    MachineFunction &MF, MachineInstr *MI, ArrayRef<unsigned> Ops,
    MachineBasicBlock::iterator InsertPt, int FrameIndex) const {

  errs() << "Asked to fold stack operand: ";  MI->print(errs());
  return nullptr;
}

MachineInstr *M6502InstrInfo::foldMemoryOperandImpl(
    MachineFunction &MF, MachineInstr *MI, ArrayRef<unsigned> Ops,
    MachineBasicBlock::iterator InsertPt, MachineInstr *LoadMI) const {

  errs() << "Asked to fold mem operand: ";  MI->print(errs());
  return nullptr;
}
