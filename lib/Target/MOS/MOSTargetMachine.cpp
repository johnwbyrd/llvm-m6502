//===-- MOSTargetMachine.cpp - Define TargetMachine for MOS -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "MOSTargetMachine.h"
#include "LeonPasses.h"
#include "MOS.h"
#include "MOSTargetObjectFile.h"
#include "TargetInfo/MOSTargetInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeMOSTarget() {
  // Register the target.
  RegisterTargetMachine<MOSV8TargetMachine> X(getTheMOSTarget());
  RegisterTargetMachine<MOSV9TargetMachine> Y(getTheMOSV9Target());
  RegisterTargetMachine<MOSelTargetMachine> Z(getTheMOSelTarget());
}

static std::string computeDataLayout(const Triple &T, bool is64Bit) {
  // MOS is typically big endian, but some are little.
  std::string Ret = T.getArch() == Triple::mosel ? "e" : "E";
  Ret += "-m:e";

  // Some ABIs have 32bit pointers.
  if (!is64Bit)
    Ret += "-p:32:32";

  // Alignments for 64 bit integers.
  Ret += "-i64:64";

  // On MOSV9 128 floats are aligned to 128 bits, on others only to 64.
  // On MOSV9 registers can hold 64 or 32 bits, on others only 32.
  if (is64Bit)
    Ret += "-n32:64";
  else
    Ret += "-f128:64-n32";

  if (is64Bit)
    Ret += "-S128";
  else
    Ret += "-S64";

  return Ret;
}

static Reloc::Model getEffectiveRelocModel(Optional<Reloc::Model> RM) {
  if (!RM.hasValue())
    return Reloc::Static;
  return *RM;
}

// Code models. Some only make sense for 64-bit code.
//
// SunCC  Reloc   CodeModel  Constraints
// abs32  Static  Small      text+data+bss linked below 2^32 bytes
// abs44  Static  Medium     text+data+bss linked below 2^44 bytes
// abs64  Static  Large      text smaller than 2^31 bytes
// pic13  PIC_    Small      GOT < 2^13 bytes
// pic32  PIC_    Medium     GOT < 2^32 bytes
//
// All code models require that the text segment is smaller than 2GB.
static CodeModel::Model
getEffectiveMOSCodeModel(Optional<CodeModel::Model> CM, Reloc::Model RM,
                           bool Is64Bit, bool JIT) {
  if (CM) {
    if (*CM == CodeModel::Tiny)
      report_fatal_error("Target does not support the tiny CodeModel", false);
    if (*CM == CodeModel::Kernel)
      report_fatal_error("Target does not support the kernel CodeModel", false);
    return *CM;
  }
  if (Is64Bit) {
    if (JIT)
      return CodeModel::Large;
    return RM == Reloc::PIC_ ? CodeModel::Small : CodeModel::Medium;
  }
  return CodeModel::Small;
}

/// Create an ILP32 architecture model
MOSTargetMachine::MOSTargetMachine(
    const Target &T, const Triple &TT, StringRef CPU, StringRef FS,
    const TargetOptions &Options, Optional<Reloc::Model> RM,
    Optional<CodeModel::Model> CM, CodeGenOpt::Level OL, bool JIT, bool is64bit)
    : LLVMTargetMachine(T, computeDataLayout(TT, is64bit), TT, CPU, FS, Options,
                        getEffectiveRelocModel(RM),
                        getEffectiveMOSCodeModel(
                            CM, getEffectiveRelocModel(RM), is64bit, JIT),
                        OL),
      TLOF(make_unique<MOSELFTargetObjectFile>()),
      Subtarget(TT, CPU, FS, *this, is64bit), is64Bit(is64bit) {
  initAsmInfo();
}

MOSTargetMachine::~MOSTargetMachine() {}

const MOSSubtarget *
MOSTargetMachine::getSubtargetImpl(const Function &F) const {
  Attribute CPUAttr = F.getFnAttribute("target-cpu");
  Attribute FSAttr = F.getFnAttribute("target-features");

  std::string CPU = !CPUAttr.hasAttribute(Attribute::None)
                        ? CPUAttr.getValueAsString().str()
                        : TargetCPU;
  std::string FS = !FSAttr.hasAttribute(Attribute::None)
                       ? FSAttr.getValueAsString().str()
                       : TargetFS;

  // FIXME: This is related to the code below to reset the target options,
  // we need to know whether or not the soft float flag is set on the
  // function, so we can enable it as a subtarget feature.
  bool softFloat =
      F.hasFnAttribute("use-soft-float") &&
      F.getFnAttribute("use-soft-float").getValueAsString() == "true";

  if (softFloat)
    FS += FS.empty() ? "+soft-float" : ",+soft-float";

  auto &I = SubtargetMap[CPU + FS];
  if (!I) {
    // This needs to be done before we create a new subtarget since any
    // creation will depend on the TM and the code generation flags on the
    // function that reside in TargetOptions.
    resetTargetOptions(F);
    I = llvm::make_unique<MOSSubtarget>(TargetTriple, CPU, FS, *this,
                                          this->is64Bit);
  }
  return I.get();
}

namespace {
/// MOS Code Generator Pass Configuration Options.
class MOSPassConfig : public TargetPassConfig {
public:
  MOSPassConfig(MOSTargetMachine &TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  MOSTargetMachine &getMOSTargetMachine() const {
    return getTM<MOSTargetMachine>();
  }

  void addIRPasses() override;
  bool addInstSelector() override;
  void addPreEmitPass() override;
};
} // namespace

TargetPassConfig *MOSTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new MOSPassConfig(*this, PM);
}

void MOSPassConfig::addIRPasses() {
  addPass(createAtomicExpandPass());

  TargetPassConfig::addIRPasses();
}

bool MOSPassConfig::addInstSelector() {
  addPass(createMOSISelDag(getMOSTargetMachine()));
  return false;
}

void MOSPassConfig::addPreEmitPass(){
  addPass(createMOSDelaySlotFillerPass());

  if (this->getMOSTargetMachine().getSubtargetImpl()->insertNOPLoad())
  {
    addPass(new InsertNOPLoad());
  }
  if (this->getMOSTargetMachine().getSubtargetImpl()->detectRoundChange()) {
    addPass(new DetectRoundChange());
  }
  if (this->getMOSTargetMachine().getSubtargetImpl()->fixAllFDIVSQRT())
  {
    addPass(new FixAllFDIVSQRT());
  }
}

void MOSV8TargetMachine::anchor() { }

MOSV8TargetMachine::MOSV8TargetMachine(const Target &T, const Triple &TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           Optional<Reloc::Model> RM,
                                           Optional<CodeModel::Model> CM,
                                           CodeGenOpt::Level OL, bool JIT)
    : MOSTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, JIT, false) {}

void MOSV9TargetMachine::anchor() { }

MOSV9TargetMachine::MOSV9TargetMachine(const Target &T, const Triple &TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           Optional<Reloc::Model> RM,
                                           Optional<CodeModel::Model> CM,
                                           CodeGenOpt::Level OL, bool JIT)
    : MOSTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, JIT, true) {}

void MOSelTargetMachine::anchor() {}

MOSelTargetMachine::MOSelTargetMachine(const Target &T, const Triple &TT,
                                           StringRef CPU, StringRef FS,
                                           const TargetOptions &Options,
                                           Optional<Reloc::Model> RM,
                                           Optional<CodeModel::Model> CM,
                                           CodeGenOpt::Level OL, bool JIT)
    : MOSTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL, JIT, false) {}
