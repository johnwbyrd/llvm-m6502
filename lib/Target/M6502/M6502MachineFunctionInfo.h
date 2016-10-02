// TODO: header stuff

#ifndef LLVM_LIB_TARGET_M6502_M6502MACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_M6502_M6502MACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

class M6502FunctionInfo final : public MachineFunctionInfo {
  // This class is adapted from WebAssemblyMachineFunctionInfo.h. WebAssembly
  // is another backend that disables LLVM's standard register allocator.
  MachineFunction &MF;

  std::vector<unsigned> M6502Regs;

public:
  explicit M6502FunctionInfo(MachineFunction &MF) : MF(MF) {}
  ~M6502FunctionInfo() override {}

  static const unsigned UnusedReg = -1u;

  void initM6502Regs();
  void setM6502Reg(unsigned VReg, unsigned M6502Reg);
  unsigned getM6502Reg(unsigned Reg) const;
  size_t getNumM6502Regs() const;
};

} // end namespace llvm

#endif
