
include "llvm/Target/Target.td"

def Mos6502InstrInfo : InstrInfo;

def Mos6502 : Target {
  let InstructionSet = Mos6502InstrInfo;
}
