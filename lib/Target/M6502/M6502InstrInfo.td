// TODO: header stuff

def M6502return : SDNode<"M6502ISD::RETURN", SDTNone,
                           [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

class I : Instruction {
  let Namespace = "M6502";
}

class Pseudo : I {
  let isPseudo = 1;
}

multiclass AccOperatorI<string AsmName, SDPatternOperator Oper> {
  // Acc, General pseudo-instruction
  def reg_pseudo : Pseudo {
    let OutOperandList = (outs Acc:$ao);
    let InOperandList = (ins Acc:$ai, General:$r);
    let Constraints = "$ao = $ai";
    let AsmString = AsmName # "_reg_pseudo $ai, $r";
    let Pattern = [(set Acc:$ao, (Oper Acc:$ai, General:$r))];
  }

  // Stack Loading pseudo-instruction
  def stack_pseudo : Pseudo {
    let OutOperandList = (outs Acc:$ao);
    let InOperandList = (ins Acc:$ai, i16imm:$idx);
    let Constraints = "$ao = $ai";
    let AsmString = AsmName # "_stack $ai, [$idx]";
    // TODO: stack loading instr pattern
    let Pattern = [(set Acc:$ao, (Oper Acc:$ai, (load imm:$idx)))];
    let mayLoad = 1;
  }

  // Immediate addressing
  def imm : I {
    let OutOperandList = (outs Acc:$ao);
    let InOperandList = (ins Acc:$ai, i8imm:$x);
    let Constraints = "$ao = $ai";
    let AsmString = AsmName # "_imm $ai, #$x";
    let Pattern = [(set Acc:$ao, (Oper Acc:$ai, imm:$x))];
    let Size = 2;
  }

  // Absolute addressing
  def abs : I {
    let OutOperandList = (outs Acc:$ao);
    let InOperandList = (ins Acc:$ai, i16imm:$addr);
    let Constraints = "$ao = $ai";
    let AsmString = AsmName # "_abs $ai, $addr";
    let Pattern = [(set Acc:$ao, (Oper Acc:$ai, (load imm:$addr)))];
    let Size = 3;
    let mayLoad = 1;
  }
}

// FIXME: there is no ADD instruction, only ADC
defm ADD : AccOperatorI<"ADD", add>;
defm AND : AccOperatorI<"AND", and>;
defm EOR : AccOperatorI<"EOR", xor>;
defm ORA : AccOperatorI<"ORA", or>;
// FIXME: there is no SUB instruction, only SBC
defm SUB : AccOperatorI<"SUB", sub>;

// Shift left by one
def ASLacc : I {
  let OutOperandList = (outs Acc:$ao);
  let InOperandList = (ins Acc:$ai);
  let AsmString = "ASL A";
  let Pattern = [(set Acc:$ao, (shl Acc:$ai, (i8 1)))];
  let Size = 1;
}

// INX, INY
def INI : I {
  let OutOperandList = (outs Index:$io);
  let InOperandList = (ins Index:$ii);
  let Constraints = "$io = $ii";
  let AsmString = "INI $ii";
  let Pattern = [(set Index:$io, (add Index:$ii, 1))];
  let Size = 1;
}

// LDA, LDX, LDY fall under this form
def LDimm : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins i8imm:$x);
  let AsmString = "LD_imm $r, #$x";
  let Pattern = [(set General:$r, (i8 imm:$x))];
  let Size = 2;
}

// LDA, LDX, LDY fall under this form
def LDabs : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins i16imm:$addr);
  let AsmString = "LD_abs $r, $addr";
  let Pattern = [(set General:$r, (load imm:$addr))];
  let Size = 3;
  let mayLoad = 1;
  let canFoldAsLoad = 1;
}

def LDstack_pseudo : Pseudo {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins i16imm:$slot);
  let AsmString = "LD_stack $r, [$slot]";
  let Pattern = [(set General:$r, (load imm:$slot))];
  let mayLoad = 1;
  let canFoldAsLoad = 1;
}

def RTS : I {
  let isReturn = 1; // This is a return instruction
  let isTerminator = 1; // This instruction is part of the terminator for a basic block
  let isBarrier = 1; // Control flow cannot fall through this instruction
  let OutOperandList = (outs);
  let InOperandList = (ins);
  let Pattern = [(M6502return)];
  let AsmString = "RTS";
  let Size = 1;
}

// STA, STX, STY all use this form
def STabs : I {
  let OutOperandList = (outs);
  let InOperandList = (ins General:$r, i16imm:$addr);
  let AsmString = "ST_abs $r, $addr";
  let Pattern = [(store General:$r, imm:$addr)];
  let Size = 3;
  let mayStore = 1;
}

def STstack_pseudo : Pseudo {
  let OutOperandList = (outs);
  let InOperandList = (ins General:$r, i16imm:$slot);
  let AsmString = "ST_stack $r, [$slot]";
  let Pattern = [(store General:$r, imm:$slot)];
  let mayStore = 1;
}

// Transfer X/Y to A
def TIA : I {
  let OutOperandList = (outs Acc:$a); // Must be A
  let InOperandList = (ins Index:$i);
  let AsmString = "TIA $i, $a";
  let Pattern = []; // NOTE: instr behavior is established in copyPhysReg
  let Size = 1;
}

// Transfer A to X/Y
def TAI : I {
  let OutOperandList = (outs Index:$i);
  let InOperandList = (ins Acc:$a); // Must be A
  let AsmString = "TAI $a, $i";
  let Pattern = []; // NOTE: instr behavior is established in copyPhysReg
  let Size = 1;
}
