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

// "Return" custom SelectionDAG Node //////

// FIXME: Really variadic? What for?
def Mos6502return : SDNode<"Mos6502ISD::RETURN", SDTNone,
                           [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

///////////////////////////////////////////

// TODO: Move to Mos6502InstrInfo.td ///

def LDA : Instruction {
  let Namespace = "Mos6502";
  let Defs = [A];
  let OutOperandList = (outs);
  let InOperandList = (ins i8imm:$x);
  let AsmString = "LDA $x";
  let Pattern = [(set A, (i8 imm:$x))];
  let Size = 2;
}

def LDX : Instruction {
  let Namespace = "Mos6502";
  let Defs = [X];
  let OutOperandList = (outs);
  let InOperandList = (ins i8imm:$x);
  let AsmString = "LDX $x";
  let Pattern = [(set X, (i8 imm:$x))];
  let Size = 2;
}

def LDY : Instruction {
  let Namespace = "Mos6502";
  let Defs = [Y];
  let OutOperandList = (outs);
  let InOperandList = (ins i8imm:$x);
  let AsmString = "LDY $x";
  let Pattern = [(set Y, (i8 imm:$x))];
  let Size = 2;
}

def RTS : Instruction {
  let Namespace = "Mos6502";
  let isReturn = 1; // This is a return instruction
  let isTerminator = 1; // This instruction is part of the terminator for a basic block
  let isBarrier = 1; // Control flow cannot fall through this instruction
  let OutOperandList = (outs);
  let InOperandList = (ins);
  let Pattern = [(Mos6502return)];
  let AsmString = "RTS";
  let Size = 1;
}

////////////////////////////////////////

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

def Mos6502InstrInfo : InstrInfo;

def Mos6502 : Target {
  let InstructionSet = Mos6502InstrInfo;
}
