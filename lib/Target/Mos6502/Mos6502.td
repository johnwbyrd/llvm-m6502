// TODO: header stuff

include "llvm/Target/Target.td"

// TODO: Move to Mos6502RegisterInfo.td ///

class Mos6502Reg<string n> : Register<n> {
  let Namespace = "Mos6502";
}

def A : Mos6502Reg<"A">;
def X : Mos6502Reg<"X">; // FIXME: X and Y are index registers
def Y : Mos6502Reg<"Y">;

def ARegs : RegisterClass<"Mos6502", [i8], 8,
  (add A, X, Y)>;

///////////////////////////////////////////

// TODO: Move to Mos6502InstrInfo.td ///

// XXX: sample from XCore
//def NOT : _F2R<0b100010, (outs GRRegs:$dst), (ins GRRegs:$b),
//                "not $dst, $b", [(set GRRegs:$dst, (not GRRegs:$b))]>;

def NOT : Instruction {
  let Namespace = "Mos6502";
  let OutOperandList = (outs ARegs:$dst);
  let InOperandList = (ins ARegs:$b);
  let AsmString = "NOT $dst, $b";
  let Pattern = [(set ARegs:$dst, (not ARegs:$b))];
  let Size = 3;
}

////////////////////////////////////////

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

def Mos6502InstrInfo : InstrInfo;

def Mos6502 : Target {
  let InstructionSet = Mos6502InstrInfo;
}
