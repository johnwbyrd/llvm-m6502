// TODO: header stuff

#include "Mos6502InstPrinter.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

void Mos6502InstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                                   StringRef Annot,
                                   const MCSubtargetInfo &STI) {

  // TODO
  O << "garblefart!\n";
}
