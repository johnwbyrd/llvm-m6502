// TODO: header stuff

#include "M6502InstPrinter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ErrorHandling.h"
using namespace llvm;

#include "M6502GenAsmWriter.inc"

void M6502InstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                                 StringRef Annot,
                                 const MCSubtargetInfo &STI) {

  // print instruction with auto-generated code from the .td files
  printInstruction(MI, O);
  printAnnotation(O, Annot);
}

void M6502InstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                    raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isImm()) {
    O << Op.getImm();
  } else if (Op.isReg()) {
    O << getRegisterName(Op.getReg());
  } else if (Op.isExpr()) {
    Op.getExpr()->print(O, &MAI, true);
  } else {
    llvm_unreachable("Invalid assembly operand");
  }
}
