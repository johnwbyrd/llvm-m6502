// TODO: header stuff

#include "Mos6502InstPrinter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#include "Mos6502GenAsmWriter.inc"

void Mos6502InstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                                   StringRef Annot,
                                   const MCSubtargetInfo &STI) {

  // print instruction with auto-generated code from the .td files
  printInstruction(MI, O);
  printAnnotation(O, Annot);
}

void Mos6502InstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                      raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isImm()) {
    O << '#' << Op.getImm();
  } else {
    // TODO: print other types of operands
    O << "*TODO*";
  }
}
