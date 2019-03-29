// TODO: header stuff

#ifndef LLVM_LIB_TARGET_M6502_M6502FUNCTIONINFO_H
#define LLVM_LIB_TARGET_M6502_M6502FUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

class M6502FunctionInfo final : public MachineFunctionInfo {
  MachineFunction &MF;

  // TODO: Put machine-specific function info here

public:
  explicit M6502FunctionInfo(MachineFunction &MF) : MF(MF) {}
  ~M6502FunctionInfo() override {}
};

} // end namespace llvm

#endif
