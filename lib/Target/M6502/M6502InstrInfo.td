// TODO: header stuff

def M6502return : SDNode<"M6502ISD::RETURN", SDTNone,
                           [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

class I : Instruction {
  let Namespace = "M6502";
}

class Pseudo : I {
  let isPseudo = 1;
}

// FIXME: There is no ADD instruction, only ADC
def ADDidx_pseudo : Pseudo {
  let OutOperandList = (outs Acc:$ao);
  let InOperandList = (ins Acc:$ai, Index:$idx);
  let Constraints = "$ao = $ai";
  let AsmString = "ADDidx_pseudo $ai, $idx";
  let Pattern = [(set Acc:$ao, (add Acc:$ai, Index:$idx))];
  //let isCommutable = 1;
}

// FIXME: There is no SUB instruction, only SBC
def SUBidx_pseudo : Pseudo {
  let OutOperandList = (outs Acc:$ao);
  let InOperandList = (ins Acc:$ai, Index:$idx);
  let Constraints = "$ao = $ai";
  let AsmString = "SUBidx_pseudo $ai, $idx";
  let Pattern = [(set Acc:$ao, (sub Acc:$ai, Index:$idx))];
}

// FIXME: There is no ADD instruction, only ADC
def ADDimm : I {
  let OutOperandList = (outs Acc:$ao);
  let InOperandList = (ins Acc:$ai, i8imm:$x);
  let Constraints = "$ao = $ai";
  let AsmString = "ADD $ai, #$x";
  let Pattern = [(set Acc:$ao, (add Acc:$ai, (i8 imm:$x)))];
  let Size = 2;
}

// FIXME: There is no ADD instruction, only ADC
def ADDabs : I {
  let OutOperandList = (outs Acc:$ao);
  let InOperandList = (ins Acc:$ai, i16imm:$addr);
  let Constraints = "$ao = $ai";
  let AsmString = "ADD $ai, $addr";
  let Pattern = [(set Acc:$ao, (add Acc:$ai, (load imm:$addr)))];
  let Size = 3;
  let mayLoad = 1;
}


// LDA, LDX, LDY fall under this form
def LDRimm : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins i8imm:$x);
  let AsmString = "LD_reg $r, #$x";
  let Pattern = [(set General:$r, (i8 imm:$x))];
  let Size = 2;
}

// LDA, LDX, LDY fall under this form
def LDRabs : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins i16imm:$addr);
  let AsmString = "LD_reg $r, $addr";
  let Pattern = [(set General:$r, (load imm:$addr))];
  let Size = 3;
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
def STRabs : I {
  let OutOperandList = (outs);
  let InOperandList = (ins General:$r, i16imm:$addr);
  let AsmString = "ST_reg $r, $addr";
  let Pattern = [(store General:$r, imm:$addr)];
  let Size = 3;
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
