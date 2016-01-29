// TODO: header stuff

#include "Mos6502MCTargetDesc.h"
#include "Mos6502MCAsmInfo.h"
#include "InstPrinter/Mos6502InstPrinter.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "Mos6502GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "Mos6502GenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "Mos6502GenRegisterInfo.inc"

static MCInstrInfo *createMos6502MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitMos6502MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createMos6502MCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitMos6502MCRegisterInfo(X, 0); /* FIXME: what's second parameter? return address? */
  return X;
}

static MCSubtargetInfo *
createMos6502MCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  return createMos6502MCSubtargetInfoImpl(TT, CPU, FS);
}

static MCAsmInfo *createMos6502MCAsmInfo(const MCRegisterInfo &MRI,
                                         const Triple &TT) {
  // TODO: Set up initial stack frame state
  return new Mos6502MCAsmInfo();
}

static MCInstPrinter *createMos6502MCInstPrinter(const Triple &T,
                                                 unsigned SyntaxVariant,
                                                 const MCAsmInfo &MAI,
                                                 const MCInstrInfo &MII,
                                                 const MCRegisterInfo &MRI) {
  return new Mos6502InstPrinter(MAI, MII, MRI);
}

extern "C" void LLVMInitializeMos6502TargetMC() {
  // Register the MCAsmInfo.
  TargetRegistry::RegisterMCAsmInfo(TheMos6502Target,
                                    createMos6502MCAsmInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheMos6502Target,
                                      createMos6502MCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheMos6502Target,
                                    createMos6502MCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheMos6502Target,
                                          createMos6502MCSubtargetInfo);

  // Register the MCInstPrinter
  TargetRegistry::RegisterMCInstPrinter(TheMos6502Target,
                                        createMos6502MCInstPrinter);
}
