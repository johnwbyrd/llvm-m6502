// TODO: header stuff

// the llvm configuration program activates Asm Printer for the target if it
// finds this file in the target subdirectory.

#include "Mos6502.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace llvm {
class MCStreamer;

class LLVM_LIBRARY_VISIBILITY Mos6502AsmPrinter : public AsmPrinter {
public:
  explicit Mos6502AsmPrinter(TargetMachine &TM,
                             std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}

  const char *getPassName() const override {
    return "Mos6502 Assembly Printer";
  }

  void EmitInstruction(const MachineInstr *MI) override;
};
} // end namespace llvm

extern "C" void LLVMInitializeMos6502AsmPrinter() {
  RegisterAsmPrinter<Mos6502AsmPrinter> X(TheMos6502Target);
}

static void LowerMachineInstrToMCInst(const MachineInstr *MI, MCInst &OutMI) {
  // TODO
  // XXX: copied from BPFMCInstLower.cpp
  OutMI.setOpcode(MI->getOpcode());

  for (unsigned i = 0, e = MI->getNumOperands(); i != e; ++i) {
    const MachineOperand &MO = MI->getOperand(i);

    MCOperand MCOp;
    switch (MO.getType()) {
    default:
      MI->dump();
      llvm_unreachable("unknown operand type");
    case MachineOperand::MO_Register:
      // Ignore all implicit register operands.
      if (MO.isImplicit())
        continue;
      MCOp = MCOperand::createReg(MO.getReg());
      break;
    case MachineOperand::MO_Immediate:
      MCOp = MCOperand::createImm(MO.getImm());
      break;
    case MachineOperand::MO_MachineBasicBlock:
      //MCOp = MCOperand::createExpr(
      //    MCSymbolRefExpr::create(MO.getMBB()->getSymbol(), Ctx));
	  assert(false && "Mos6502 does not implement MO_MachineBasicBlock operands");
      break;
    case MachineOperand::MO_RegisterMask:
      continue;
    case MachineOperand::MO_GlobalAddress:
      //MCOp = LowerSymbolOperand(MO, GetGlobalAddressSymbol(MO));
	  assert(false && "Mos6502 does not implement MO_GlobalAddress operands");
      break;
    }

    OutMI.addOperand(MCOp);
  }
}

void Mos6502AsmPrinter::EmitInstruction(const MachineInstr *MI) {
  // TODO
  MCInst TmpInst;
  LowerMachineInstrToMCInst(MI, TmpInst);
  EmitToStreamer(*OutStreamer, TmpInst);
}
