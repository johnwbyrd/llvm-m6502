// TODO: header stuff

#include "M6502InstPrinter.h"
#include "M6502MachineFunctionInfo.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/ErrorHandling.h"
using namespace llvm;

#define DEBUG_TYPE "m6502-asm-printer"

#include "M6502GenAsmWriter.inc"

void M6502InstPrinter::printInst(const MCInst *MI, raw_ostream &O,
                                 StringRef Annot,
                                 const MCSubtargetInfo &STI) {
  // print instruction with auto-generated code from the .td files
  printInstruction(MI, O);
  printAnnotation(O, Annot);
}

void M6502InstPrinter::printRegName(raw_ostream &OS, unsigned RegNo) const {
  assert(RegNo != M6502FunctionInfo::UnusedReg);
  OS << "R" << RegNo;
}

void M6502InstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                    raw_ostream &O) {
  const MCOperand &Op = MI->getOperand(OpNo);
  if (Op.isImm()) {
    O << "#$";
    O.write_hex(Op.getImm());
  } else if (Op.isReg()) {
    printRegName(O, Op.getReg());
  } else if (Op.isExpr()) {
    Op.getExpr()->print(O, &MAI, true);
  } else {
    llvm_unreachable("Invalid assembly operand");
  }
}

