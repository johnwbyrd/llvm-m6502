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
