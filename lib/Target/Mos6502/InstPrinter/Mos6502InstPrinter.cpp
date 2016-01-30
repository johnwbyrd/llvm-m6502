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
#if 0
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isReg()) {
    O << getRegisterName(Op.getReg());
  } else if (Op.isImm()) {
    O << (int)Op.getImm();
  } else {
    Op.getExpr()->print(O, &MAI);
  }
#else
  O << "TODO";
#endif
}
