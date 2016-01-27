// TODO: header stuff

// the llvm configuration program activates Asm Printer for the target if it
// finds this file in the target subdirectory.

#include "Mos6502.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

namespace llvm {
class MCStreamer;

class LLVM_LIBRARY_VISIBILITY Mos6502AsmPrinter : public AsmPrinter {
public:
  explicit Mos6502AsmPrinter(TargetMachine &TM,
                             std::unique_ptr<MCStreamer> Streamer)
      : AsmPrinter(TM, std::move(Streamer)) {}
};
} // end namespace llvm

extern "C" void LLVMInitializeMos6502AsmPrinter() {
  RegisterAsmPrinter<Mos6502AsmPrinter> X(TheMos6502Target);
}
