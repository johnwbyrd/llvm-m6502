// TODO: header stuff

// The following is shamelessly borrowed from Hexagon backend.
// This complex pattern exists only to create a machine instruction operand
// of type "frame index". There doesn't seem to be a way to do that directly
// in the patterns.
def AddrFI : ComplexPattern<i16, 1, "SelectAddrFI", [frameindex], []>;

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
    let InOperandList = (ins Acc:$ai, i16imm:$slot, i16imm:$offs);
    let Constraints = "$ao = $ai";
    let AsmString = AsmName # "_stack $ai, [$slot] + $offs";
    // TODO: stack loading instr pattern
    let Pattern = [(set Acc:$ao, (Oper Acc:$ai, (load (add AddrFI:$slot, imm:$offs))))];
    let mayLoad = 1;
  }

  // Immediate addressing
  def imm : I {
    let OutOperandList = (outs Acc:$ao);
    let InOperandList = (ins Acc:$ai, i8imm:$x);
    let Constraints = "$ao = $ai";
    let AsmString = AsmName # "_imm $ai, $x";
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

// DEX, DEY
def DEI : I {
  let OutOperandList = (outs Index:$io);
  let InOperandList = (ins Index:$ii);
  let Constraints = "$io = $ii";
  let AsmString = "DEI $ii";
  let Pattern = [(set Index:$io, (sub Index:$ii, 1))];
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
  let InOperandList = (ins i16imm:$slot, i16imm:$offs);
  let AsmString = "LD_stack $r, [$slot] + $offs";
  let Pattern = [(set General:$r, (load (add AddrFI:$slot, imm:$offs)))];
  let mayLoad = 1;
  let canFoldAsLoad = 1;
}

def : Pat<(i8 (load AddrFI:$slot)), (LDstack_pseudo $slot, 0)>;

// XXX: Fake store-to-pointer pseudo-instruction that takes a General reg as
//      an address.
def STToPtr_pseudo : Pseudo {
  let OutOperandList = (outs);
  let InOperandList = (ins General:$r, General:$p, i8imm:$offs);
  let AsmString = "STToPtr_psuedo $r, $p + $offs";
  let Pattern = [(store General:$r, (add General:$p, imm:$offs))];
  let mayStore = 1;
}

def : Pat<(store General:$r, General:$p), (STToPtr_pseudo $r, $p, 0)>;

// XXX: hacky pseudo-instructions to support 16-bit pointers.
//      currently unused.

/*
// Load 16-bit pointer from stack
def LDPtrFromStack_pseudo : Pseudo {
  let OutOperandList = (outs Ptr:$p);
  let InOperandList = (ins i16imm:$slot, i16imm:$offs);
  let AsmString = "LDPtrFromStack_pseudo $p, [$slot] + $offs";
  //let Pattern = [(set Ptr:$p, (load (add AddrFI:$slot, imm:$offs)))];
  // XXX: Do not pattern match on this instruction. This instruction should be
  // generated exclusively by loadRegFromStackSlot.
  let Pattern = [];
  let mayLoad = 1;
}

// Store 16-bit pointer to stack
def STPtrToStack_pseudo : Pseudo {
  let OutOperandList = (outs);
  let InOperandList = (ins Ptr:$p, i16imm:$slot, i16imm:$offs);
  let AsmString = "STPtrToStack_pseudo $p, [$slot] + $offs";
  //let Pattern = [(store Ptr:$p, (add AddrFI:$slot, imm:$offs))];
  // XXX: Do not pattern match on this instruction. This instruction should be
  // generated exclusively by storeRegToStackSlot.
  let Pattern = [];
  let mayStore = 1;
}

// Store to pointer
def STToPtr_pseudo : Pseudo {
  let OutOperandList = (outs);
  let InOperandList = (ins General:$r, Ptr:$p);
  let AsmString = "STToPtr_pseudo $r, $p";
  let Pattern = [(store General:$r, Ptr:$p)];
  let mayStore = 1;
}

// Store to pointer plus constant offset
def STToPtrOffs_pseudo : Pseudo {
  let OutOperandList = (outs);
  let InOperandList = (ins General:$r, Ptr:$p, i16imm:$offs);
  let AsmString = "STToPtrOffs_pseudo $r, $p + $offs";
  let Pattern = [(store General:$r, (add Ptr:$p, imm:$offs))];
  let mayStore = 1;
}

def SDTBuildPair : SDTypeProfile<1, 2, [SDTCisVT<0, i16>, SDTCisVT<1, i8>, SDTCisVT<2, i8>]>;
def buildpair    : SDNode<"ISD::BUILD_PAIR", SDTBuildPair>;
def BUILDptr_pseudo : Pseudo {
  let OutOperandList = (outs Ptr:$p);
  let InOperandList = (ins General:$a, General:$b);
  let AsmString = "BUILDptr_pseudo $p, $a, $b";
  let Pattern = [(set Ptr:$p, (i16 (buildpair General:$a, General:$b)))];
}
*/
/*
def ADDptrimm_pseudo : Pseudo {
  let OutOperandList = (outs Ptr:$po);
  let InOperandList = (ins Ptr:$pi, i16imm:$i);
  let AsmString = "ADDptrimm_pseudo $po, $pi, $i";
  let Pattern = [(set Ptr:$po, (add Ptr:$pi, imm:$i))];
}
*/

def LSRacc : I {
  let OutOperandList = (outs Acc:$ao);
  let InOperandList = (ins Acc:$ai);
  let AsmString = "LSR A";
  let Pattern = [(set Acc:$ao, (srl Acc:$ai, (i8 1)))];
  let Size = 1;
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
  let InOperandList = (ins General:$r, i16imm:$slot, i16imm:$offs);
  let AsmString = "ST_stack $r, [$slot] + $offs";
  let Pattern = [(store General:$r, (add AddrFI:$slot, imm:$offs))];
  let mayStore = 1;
}

def : Pat<(store General:$r, AddrFI:$slot), (STstack_pseudo $r, $slot, 0)>;

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
