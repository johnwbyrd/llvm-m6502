// TODO: header stuff

include "llvm/Target/Target.td"

// TODO: Move to Mos6502RegisterInfo.td ///

class Mos6502Reg<string n> : Register<n> {
  let Namespace = "Mos6502";
}

def A : Mos6502Reg<"A">;
def X : Mos6502Reg<"X">; // FIXME: X and Y are index registers
def Y : Mos6502Reg<"Y">;

// XXX: This is a fake register definition to enable 16-bit pointers in
// instructions such as LDA, ADD, etc.
// This register does not actually exist; there are no instructions for loading
// or storing it.
def XXXFake16 : Mos6502Reg<"XXXFake16">;

def Acc : RegisterClass<"Mos6502", [i8], 8,
  (add A)>;

def Index : RegisterClass<"Mos6502", [i8], 8,
  (add X, Y)>;

def Reg : RegisterClass<"Mos6502", [i8], 8,
  (add A, X, Y)>;

def Ptr : RegisterClass<"Mos6502", [i16], 16,
  (add XXXFake16)>;

///////////////////////////////////////////

// "Return" custom SelectionDAG Node //////

def Mos6502return : SDNode<"Mos6502ISD::RETURN", SDTNone,
                           [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

///////////////////////////////////////////

// TODO: Move to Mos6502InstrInfo.td ///

// FIXME: There is no ADD instruction, only ADC
def ADDimm : Instruction {
  let Namespace = "Mos6502";
  let OutOperandList = (outs Acc:$ao);
  let InOperandList = (ins Acc:$ai, i8imm:$x);
  let AsmString = "ADD $ao, $ai, #$x";
  let Pattern = [(set Acc:$ao, (add Acc:$ai, (i8 imm:$x)))];
  let Size = 2;
}

// FIXME: There is no add instruction, only ADC
def ADDabs : Instruction {
  let Namespace = "Mos6502";
  let OutOperandList = (outs Acc:$ao);
  let InOperandList = (ins Acc:$ai, i16imm:$addr);
  let AsmString = "ADD $ao, $ai, $addr";
  let Pattern = [(set Acc:$ao, (add Acc:$ai, (load imm:$addr)))];
  let Size = 3;
  let mayLoad = 1;
}


// LDA, LDX, LDY fall under this form
// FIXME: Pseudo-instruction?
def LDRimm : Instruction {
  let Namespace = "Mos6502";
  let OutOperandList = (outs Reg:$r);
  let InOperandList = (ins i8imm:$x);
  let AsmString = "LD_reg $r, #$x";
  let Pattern = [(set Reg:$r, (i8 imm:$x))];
  let Size = 2;
}

// LDA, LDX, LDY fall under this form
// FIXME: Pseudo-instruction?
def LDRabs : Instruction {
  let Namespace = "Mos6502";
  let OutOperandList = (outs Reg:$r);
  let InOperandList = (ins i16imm:$addr);
  let AsmString = "LD_reg $r, $addr";
  let Pattern = [(set Reg:$r, (load imm:$addr))];
  let Size = 3;
  let mayLoad = 1;
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

// STA, STX, STY all use this form
// FIXME: This ought to be a pseudo-instruction, eh?
def STRabs : Instruction {
  let Namespace = "Mos6502";
  let OutOperandList = (outs);
  let InOperandList = (ins Reg:$r, i16imm:$addr);
  let AsmString = "ST_reg $r, $addr";
  let Pattern = [(store Reg:$r, imm:$addr)];
  let Size = 3;
  let mayStore = 1;
}

// Transfer X/Y to A
// FIXME: pseudo-instruction?
def TIA : Instruction {
  let Namespace = "Mos6502";
  let OutOperandList = (outs Acc:$a); // Must be A
  let InOperandList = (ins Index:$i);
  let AsmString = "TIA $i, $a";
  let Pattern = [(set Acc:$a, Index:$i)];
  let Size = 1;
}

// Transfer A to X/Y
// FIXME: pseudo-instruction?
def TAI : Instruction {
  let Namespace = "Mos6502";
  let OutOperandList = (outs Index:$i);
  let InOperandList = (ins Acc:$a); // Must be A
  let AsmString = "TAI $a, $i";
  let Pattern = [(set Index:$i, Acc:$a)];
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
