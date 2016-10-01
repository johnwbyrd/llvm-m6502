// TODO: header stuff

// This file implements a pass which assigns M6502 virtual register numbers for
// CodeGen virtual registers. This is needed because the M6502 backend disables
// LLVM's standard register allocator. Since CodeGen virtual registers are
// not allowed by late LLVM passes, we convert them to M6502 virtual registers
// to trick late passes into thinking the registers are not virtual anymore.
// TODO: perhaps a more elegant solution could be found.

// Adapted from WebAssemblyRegNumbering.cpp. WebAssembly is another backend
// that disables LLVM's standard register allocator.

// TODO: This pass may not be necessary. Depending on the design of the
// late-compilation pass that emits true 6502 instructions, it may be possible
// to remove this pass entirely.
// For now, this pass helps generate pleasant assembly output by assigning
// reasonable numbers to virtual registers.

#include "M6502.h"
#include "M6502MachineFunctionInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
using namespace llvm;

#define DEBUG_TYPE "m6502-reg-numbering"

namespace {
class M6502RegNumbering final : public MachineFunctionPass {
  const char *getPassName() const override {
    return "M6502 Register Numbering";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

public:
  static char ID;
  M6502RegNumbering() : MachineFunctionPass(ID) {}
};
} // end anonymous namespace

char M6502RegNumbering::ID = 0;
FunctionPass *llvm::createM6502RegNumbering() {
  return new M6502RegNumbering();
}

// Adapted from WebAssemblyRegNumbering.cpp. WebAssembly is another backend
// that disables LLVM's standard register allocator. This pass assigns numbers
// to each virtual register in a machine function.
bool M6502RegNumbering::runOnMachineFunction(MachineFunction &MF) {
  DEBUG(dbgs() << "********** Register Numbering **********\n"
                  "********** Function: "
               << MF.getName() << '\n');

  M6502FunctionInfo &MFI = *MF.getInfo<M6502FunctionInfo>();
  MachineRegisterInfo &MRI = MF.getRegInfo();

  MFI.initM6502Regs();

  // NOTE: Unlike WebAssembly, M6502 does not store function arguments in
  // registers. Instead, arguments are stored on the stack.

  unsigned NumVRegs = MRI.getNumVirtRegs();
  unsigned CurReg = 0;
  for (unsigned VRegIdx = 0; VRegIdx < NumVRegs; ++VRegIdx) {
    unsigned VReg = TargetRegisterInfo::index2VirtReg(VRegIdx);
    // Skip unused registers.
    if (MRI.use_empty(VReg))
      continue;
    // NOTE: M6502 does not have a concept of "stackified" registers.
    if (MFI.getM6502Reg(VReg) == M6502FunctionInfo::UnusedReg) {
      DEBUG(dbgs() << "VReg " << VReg << " -> M6502Reg " << CurReg << "\n");
      MFI.setM6502Reg(VReg, CurReg);
      ++CurReg;
    }
  }

  return true;
}