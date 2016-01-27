// TODO: header stuff

#include "Mos6502MCTargetDesc.h"
#include "Mos6502MCAsmInfo.h"
#include "InstPrinter/Mos6502InstPrinter.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

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

  // Register the MCInstPrinter
  TargetRegistry::RegisterMCInstPrinter(TheMos6502Target,
                                        createMos6502MCInstPrinter);
}
