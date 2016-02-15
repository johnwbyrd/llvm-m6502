// TODO: license stuff

#include "M6502TargetMachine.h"
#include "M6502.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/ADT/StringRef.h"
using namespace llvm;

namespace llvm {
extern Target TheM6502Target;
}

extern "C" void LLVMInitializeM6502Target() {
  RegisterTargetMachine<M6502TargetMachine> X(TheM6502Target);
}

M6502TargetMachine::M6502TargetMachine(const Target &T, const Triple &TT,
                                       StringRef CPU, StringRef FS,
                                       const TargetOptions &Options,
                                       Reloc::Model RM, CodeModel::Model CM,
                                       CodeGenOpt::Level OL)
    : LLVMTargetMachine(T, "e-p:16:8", TT, CPU, FS, Options, RM, CM, OL),
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

  bool addInstSelector() override;
  
  /// This method may be implemented by targets that want to run passes
  /// immediately before register allocation.
  virtual void addPreRegAlloc() override;
};
} // namespace

TargetPassConfig *M6502TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new M6502PassConfig(this, PM);
}

bool M6502PassConfig::addInstSelector() {
  addPass(createM6502ISelDag(getM6502TargetMachine(), getOptLevel()));
  return false;
}

void M6502PassConfig::addPreRegAlloc() {
  addPass(createExpandAccPseudoPass());
}
