// TODO: license stuff

#include "M6502TargetMachine.h"
#include "M6502.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/ADT/StringRef.h"
using namespace llvm;

#define DEBUG_TYPE "m6502";

namespace llvm {
extern Target TheM6502Target;
}

extern "C" void LLVMInitializeM6502Target() {
  RegisterTargetMachine<M6502TargetMachine> X(TheM6502Target);
}

M6502TargetMachine::M6502TargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       Optional<Reloc::Model> RM,
                                       CodeModel::Model CM, CodeGenOpt::Level OL)
    : LLVMTargetMachine(T, "e-p:16:8-n8", TT, CPU, FS, Options,
                        RM.getValueOr(Reloc::Model::Static), CM, OL),
      TLOF(std::make_unique<M6502TargetObjectFile>()),
      Subtarget(TT, CPU, FS, *this) {

  initAsmInfo();
}

namespace {
class M6502PassConfig : public TargetPassConfig {
public:
  M6502PassConfig(M6502TargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  M6502TargetMachine &getM6502TargetMachine() const {
    return getTM<M6502TargetMachine>();
  }

  FunctionPass *createTargetRegisterAllocator(bool Optimized) override;

  bool addInstSelector() override;
  void addPostRegAlloc() override;
  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *M6502TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new M6502PassConfig(this, PM);
}

FunctionPass *M6502PassConfig::createTargetRegisterAllocator(bool Optimized) {
  // M6502 does not use LLVM's standard register allocator.
  return nullptr; // No reg alloc
}

bool M6502PassConfig::addInstSelector() {
  TargetPassConfig::addInstSelector();
  addPass(createM6502ISelDag(getM6502TargetMachine(), getOptLevel()));
  return false;
}

void M6502PassConfig::addPostRegAlloc() {
  // Adapted from WebAssemblyTargetMachine.cpp. WebAssembly is another backend
  // that disables LLVM's standard register allocator.
  
  // TODO: The following CodeGen passes don't currently support code containing
  // virtual registers. Consider removing their restrictions and re-enabling
  // them.

  // Has no asserts of its own, but was not written to handle virtual regs.
  disablePass(&ShrinkWrapID);

  // These functions all require the AllVRegsAllocated property.
  disablePass(&MachineCopyPropagationID);
  disablePass(&PostRASchedulerID);
  disablePass(&FuncletLayoutID);
  disablePass(&StackMapLivenessID);
  disablePass(&LiveDebugValuesID);
  disablePass(&PatchableFunctionID);

  addPrintPass("This banner is humble.");

  TargetPassConfig::addPostRegAlloc();
}

void M6502PassConfig::addPreEmitPass() {
  TargetPassConfig::addPreEmitPass();

  addPass(createM6502RegNumbering());
}
