// TODO: header stuff

#include "M6502MCTargetDesc.h"
#include "M6502MCAsmInfo.h"
#include "InstPrinter/M6502InstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "M6502GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "M6502GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "M6502GenRegisterInfo.inc"

static MCInstrInfo *createM6502MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitM6502MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createM6502MCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitM6502MCRegisterInfo(X, 0); /* FIXME: what's second parameter? return address? */
  return X;
}

static MCSubtargetInfo *
createM6502MCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createM6502MCSubtargetInfoImpl(TT, CPU, FS);
}

static MCAsmInfo *createM6502MCAsmInfo(const MCRegisterInfo &MRI,
                                         const Triple &TT) {
  // TODO: Set up initial stack frame state
  return new M6502MCAsmInfo();
}

static MCInstPrinter *createM6502MCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  return new M6502InstPrinter(MAI, MII, MRI);
}

extern "C" void LLVMInitializeM6502TargetMC() {
  // Register the MCAsmInfo.
  TargetRegistry::RegisterMCAsmInfo(TheM6502Target,
                                    createM6502MCAsmInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheM6502Target,
                                      createM6502MCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheM6502Target,
                                    createM6502MCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheM6502Target,
                                          createM6502MCSubtargetInfo);

  // Register the MCInstPrinter
  TargetRegistry::RegisterMCInstPrinter(TheM6502Target,
                                        createM6502MCInstPrinter);
}
