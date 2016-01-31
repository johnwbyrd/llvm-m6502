// TODO: header stuff

include "llvm/Target/Target.td"

// TODO: Move to Mos6502RegisterInfo.td ///

class Mos6502Reg<string n> : Register<n> {
  let Namespace = "Mos6502";
}

def A : Mos6502Reg<"A">;
def X : Mos6502Reg<"X">; // FIXME: X and Y are index registers
def Y : Mos6502Reg<"Y">;

// XXX: Fake register to support 16-bit pointer immediate operands in
// instructions such as LDA, ADD, etc.
// This register does not actually exist; there are no instructions for loading
// from or storing to it.
def XXXFake16 : Mos6502Reg<"XXXFake16">;

// FIXME: Should A, X and Y be the same or different register classes?
def ARegs : RegisterClass<"Mos6502", [i8], 8,
  (add A, X, Y)>;

def PtrReg : RegisterClass<"Mos6502", [i16], 16,
  (add XXXFake16)>;

///////////////////////////////////////////

// "Return" custom SelectionDAG Node //////

// FIXME: Really variadic? What for?
def Mos6502return : SDNode<"Mos6502ISD::RETURN", SDTNone,
                           [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

///////////////////////////////////////////

// TODO: Move to Mos6502InstrInfo.td ///

// FIXME: There is no ADD instruction, only ADC
def ADD_imm : Instruction {
  let Namespace = "Mos6502";
  let Defs = [A];
  let Uses = [A];
  let OutOperandList = (outs);
  let InOperandList = (ins i8imm:$x);
  let AsmString = "ADD #$x";
  let Pattern = [(set A, (add A, (i8 imm:$x)))];
  let Size = 2;
}

// FIXME: There is no add instruction, only ADC
def ADD_abs : Instruction {
  let Namespace = "Mos6502";
  let Defs = [A];
  let Uses = [A];
  let OutOperandList = (outs);
  let InOperandList = (ins i16imm:$addr); // FIXME: addr is a 1-byte offset within the 0 page
  let AsmString = "ADD $addr";
  let Pattern = [(set A, (add A, (load imm:$addr)))];
  let Size = 3;
  let mayLoad = 1;
}

def LDA_imm : Instruction {
  let Namespace = "Mos6502";
  let Defs = [A];
  let OutOperandList = (outs);
  let InOperandList = (ins i8imm:$x);
  let AsmString = "LDA #$x";
  let Pattern = [(set A, (i8 imm:$x))];
  let Size = 2;
}

def LDA_abs : Instruction {
  let Namespace = "Mos6502";
  let Defs = [A];
  let OutOperandList = (outs);
  let InOperandList = (ins i16imm:$addr);
  let AsmString = "LDA $addr";
  let Pattern = [(set A, (load imm:$addr))];
  let Size = 3;
  let mayLoad = 1;
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

// Transfer A to X
def TAX : Instruction {
  let Namespace = "Mos6502";
  let Defs = [X];
  let Uses = [A];
  let OutOperandList = (outs);
  let InOperandList = (ins);
  let AsmString = "TAX";
  let Pattern = [(set X, A)];
  let Size = 1;
}

// Transfer A to Y
def TAY : Instruction {
  let Namespace = "Mos6502";
  let Defs = [Y];
  let Uses = [A];
  let OutOperandList = (outs);
  let InOperandList = (ins);
  let AsmString = "TAY";
  let Pattern = [(set Y, A)];
  let Size = 1;
}

// Transfer X to A
def TXA : Instruction {
  let Namespace = "Mos6502";
  let Defs = [A];
  let Uses = [X];
  let OutOperandList = (outs);
  let InOperandList = (ins);
  let AsmString = "TXA";
  let Pattern = [(set A, X)];
  let Size = 1;
}

// Transfer Y to A
def TYA : Instruction {
  let Namespace = "Mos6502";
  let Defs = [A];
  let Uses = [Y];
  let OutOperandList = (outs);
  let InOperandList = (ins);
  let AsmString = "TYA";
  let Pattern = [(set A, Y)];
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
