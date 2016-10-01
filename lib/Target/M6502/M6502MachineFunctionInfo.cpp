// TODO: header stuff

#include "M6502MachineFunctionInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
using namespace llvm;

void M6502FunctionInfo::initM6502Regs() {
  assert(M6502Regs.empty());
  unsigned Reg = UnusedReg;
  M6502Regs.resize(MF.getRegInfo().getNumVirtRegs(), Reg);
}

void M6502FunctionInfo::setM6502Reg(unsigned VReg, unsigned M6502Reg) {
  assert(M6502Reg != UnusedReg);
  assert(TargetRegisterInfo::virtReg2Index(VReg) < M6502Regs.size());
  M6502Regs[TargetRegisterInfo::virtReg2Index(VReg)] = M6502Reg;
}

unsigned M6502FunctionInfo::getM6502Reg(unsigned Reg) const {
  assert(TargetRegisterInfo::virtReg2Index(Reg) < M6502Regs.size());
  return M6502Regs[TargetRegisterInfo::virtReg2Index(Reg)];
}
