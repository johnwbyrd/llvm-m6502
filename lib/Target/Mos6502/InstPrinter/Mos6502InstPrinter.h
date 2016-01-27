// TODO: header stuff

#ifndef LLVM_LIB_TARGET_MOS6502_INSTPRINTER_MOS6502INSTPRINTER_H
#define LLVM_LIB_TARGET_MOS6502_INSTPRINTER_MOS6502INSTPRINTER_H

#include "llvm/MC/MCInstPrinter.h"

namespace llvm {

class Mos6502InstPrinter : public MCInstPrinter {
public:
  Mos6502InstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                   const MCRegisterInfo &MRI)
      : MCInstPrinter(MAI, MII, MRI) {}

  void printInst(const MCInst *MI, raw_ostream &O, StringRef Annot,
                 const MCSubtargetInfo &STI) override;
};

} // end namespace llvm

#endif
