// TODO: header stuff

// Custom operand types

// XXX: this seems to be missing from Target.td
let OperandType = "OPERAND_IMMEDIATE" in {
  //def iPTRimm : Operand<iPTR>; // FIXME: this breaks.
  def iPTRimm : Operand<i16>;
}

def brtarget : Operand<OtherVT>;

// Custom patterns

// The following is shamelessly borrowed from Hexagon backend.
// This complex pattern creates a machine instruction operand of type
// "frameindex". There doesn't seem to be a way to do that directly
// in the patterns.
def AddrFI : ComplexPattern<iPTR, 1, "SelectAddrFI", [frameindex], []>;

// Custom type profiles

def SDT_M6502Cmp : SDTypeProfile<0, 2, [SDTCisVT<0, i8>, SDTCisVT<1, i8>]>;
// FIXME: in SDT_M6502BSet, is first parameter correct for a Flag register?
def SDT_M6502BSet : SDTypeProfile<0, 2,
                                  [SDTCisInt<0>, SDTCisVT<1, OtherVT>]>;
// FIXME: in SDT_M6502BClear, is first parameter correct for a Flag register?
def SDT_M6502BClear : SDTypeProfile<0, 2,
                                    [SDTCisInt<0>, SDTCisVT<1, OtherVT>]>;
def SDT_M6502PtrHi : SDTypeProfile<1, 1,
                                   [SDTCisVT<0, i8>, SDTCisVT<1, i16>]>;
def SDT_M6502PtrLo : SDTypeProfile<1, 1,
                                   [SDTCisVT<0, i8>, SDTCisVT<1, i16>]>;
def SDT_M6502BuildPtr : SDTypeProfile<1, 2,
                                      [SDTCisVT<0, i16>, SDTCisVT<1, i8>,
                                       SDTCisVT<2, i8>]>;

// Custom nodes

def M6502return : SDNode<"M6502ISD::RETURN", SDTNone,
                         [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def M6502cmp : SDNode<"M6502ISD::CMP", SDT_M6502Cmp, [SDNPOutGlue]>;
def M6502bset : SDNode<"M6502ISD::BSET", SDT_M6502BSet,
                       [SDNPHasChain, SDNPInGlue, SDNPOutGlue]>;
def M6502bclear : SDNode<"M6502ISD::BCLEAR", SDT_M6502BClear,
                         [SDNPHasChain, SDNPInGlue, SDNPOutGlue]>;
def M6502ptrhi : SDNode<"M6502ISD::PTRHI", SDT_M6502PtrHi, []>;
def M6502ptrlo : SDNode<"M6502ISD::PTRLO", SDT_M6502PtrLo, []>;
def M6502buildptr : SDNode<"M6502ISD::BUILDPTR", SDT_M6502BuildPtr, []>;

// Instructions

class I : Instruction {
  let Namespace = "M6502";
  let isPseudo = 1; // XXX: all instructions are pseudo-instructions for now.
}

multiclass AccOperatorI<string AsmName, SDPatternOperator Oper> {
  // Register operand (psuedo)
  def _reg : I {
    let OutOperandList = (outs General:$ao);
    let InOperandList = (ins General:$ai, General:$r);
    let Constraints = "$ao = $ai";
    let AsmString = AsmName # "_reg $ai, $r";
    let Pattern = [(set General:$ao, (Oper General:$ai, General:$r))];
  }

  // Stack operand (pseudo)
  def _stack : I {
    let OutOperandList = (outs General:$ao);
    let InOperandList = (ins General:$ai, iPTRimm:$slot, iPTRimm:$offs);
    let Constraints = "$ao = $ai";
    let AsmString = AsmName # "_stack $ai, [$slot]+$offs";
    // TODO: stack loading instr pattern
    let Pattern = [(set General:$ao, (Oper General:$ai, (load (add AddrFI:$slot, imm:$offs))))];
    let mayLoad = 1;
  }

  // Immediate
  def _imm : I {
    let OutOperandList = (outs General:$ao);
    let InOperandList = (ins General:$ai, i8imm:$x);
    let Constraints = "$ao = $ai";
    let AsmString = AsmName # "_imm $ai, $x";
    let Pattern = [(set General:$ao, (Oper General:$ai, imm:$x))];
    let Size = 2;
  }

  // Absolute addressing
  def _abs : I {
    let OutOperandList = (outs General:$ao);
    let InOperandList = (ins General:$ai, iPTRimm:$addr);
    let Constraints = "$ao = $ai";
    let AsmString = AsmName # "_abs $ai, $addr";
    let Pattern = [(set General:$ao, (Oper General:$ai, (load imm:$addr)))];
    let Size = 3;
    let mayLoad = 1;
  }
}

// XXX: Fake "AD0" instruction should generate:
//      CLC
//      ADC ...
let Defs = [CF] in
  defm AD0 : AccOperatorI<"AD0", addc>;
let Defs = [CF], Uses = [CF] in
  defm ADC : AccOperatorI<"ADC", adde>;

// XXX: Fake instruction for regular add
defm ADD : AccOperatorI<"ADD", add>;

// XXX: Fake "SB1" instruction should generate:
//      SEC
//      SBC ...
let Defs = [CF] in
  defm SB1 : AccOperatorI<"SB1", subc>;
let Defs = [CF], Uses = [CF] in
  defm SBC : AccOperatorI<"SBC", sube>;

// XXX: Fake instruction for regular sub
defm SUB : AccOperatorI<"SUB", sub>;

defm AND : AccOperatorI<"AND", and>;
defm EOR : AccOperatorI<"EOR", xor>;
defm ORA : AccOperatorI<"ORA", or>;

// Shift left by one
def ASL_reg : I {
  let OutOperandList = (outs General:$ao);
  let InOperandList = (ins General:$ai);
  let AsmString = "ASL $ai";
  let Pattern = [(set General:$ao, (shl General:$ai, (i8 1)))];
  let Size = 1;
}

// Branch on flag clear
//   BPL: branch on NF = 0
//   BNE: branch on ZF = 0
//   BCC: branch on CF = 0
//   BVC: branch on VF = 0
// TODO: set Uses = [*Flag] on conditional branch instructions
def BCLEAR : I {
  let OutOperandList = (outs);
  let InOperandList = (ins Flag:$f, brtarget:$target);
  let AsmString = "BCLEAR $f, $target"; // TODO: print correct mnemonic.
  let Pattern = [(M6502bclear (i1 Flag:$f), bb:$target)];
  // FIXME: conditional branch instructions can only target a relative address
  // within -128..127 bytes of the program counter. Extra handling is needed for
  // other addresses.
  let Size = 2;
  let isBranch = 1; // This is a branch instruction
  let isTerminator = 1; // This instruction is part of the terminator for a basic block
}

// Branch on flag set
//   BMI: branch on NF = 1
//   BEQ: branch on ZF = 1
//   BCS: branch on CF = 1
//   BVS: branch on VF = 1
// TODO: set Uses = [*Flag] on conditional branch instructions
def BSET : I {
  let OutOperandList = (outs);
  let InOperandList = (ins Flag:$f, brtarget:$target);
  let AsmString = "BSET $f, $target"; // TODO: print correct mnemonic.
  let Pattern = [(M6502bset (i1 Flag:$f), bb:$target)];
  // FIXME: conditional branch instructions can only target a relative address
  // within -128..127 bytes of the program counter. Extra handling is needed for
  // other addresses.
  let Size = 2;
  let isBranch = 1; // This is a branch instruction
  let isTerminator = 1; // This instruction is part of the terminator for a basic block
}

// Compare Immediate with Accumulator
// TODO: Define CMP with a multiclass like AccOperatorI
def CMP_imm : I {
  let Defs = [NF, ZF, CF];
  let OutOperandList = (outs);
  let InOperandList = (ins General:$ai, i8imm:$x);
  //let Constraints = "";
  let AsmString = "CMP_imm $ai, $x";
  let Pattern = [(M6502cmp General:$ai, imm:$x)];
  let Size = 2;
}

// Compare Memory with Accumulator
// TODO: Define CMP with a multiclass like AccOperatorI
def CMP_stack : I {
  let Defs = [NF, ZF, CF];
  let OutOperandList = (outs);
  let InOperandList = (ins General:$ai, iPTRimm:$slot, iPTRimm:$offs);
  //let Constraints = "";
  let AsmString = "CMP_stack $ai, [$slot]+$offs";
  let Pattern = [(M6502cmp General:$ai, (load (add AddrFI:$slot, imm:$offs)))];
  let mayLoad = 1;
}

// TODO: Define CMP with a multiclass like AccOperatorI
// XXX: this is a desperate hack to get stack-load operand to pattern-match...
// ...CMP_stack doesn't match (M6502cmp Acc, (load AddrFI)) because
// because the second operand is not (add AddrFI, imm).
// If there is no offset added to AddrFI then CMP_stack will not
// pattern-match.
// The following is a version of CMP_stack without the offset.
// FIXME: stack variables, in general, need rethinking.
def CMP_stackXXX : I {
  let Defs = [NF, ZF, CF];
  let OutOperandList = (outs);
  let InOperandList = (ins General:$ai, iPTRimm:$slot);
  //let Constraints = "";
  let AsmString = "CMP_stackXXX $ai, [$slot]+0";
  let Pattern = [(M6502cmp General:$ai, (load AddrFI:$slot))];
  let mayLoad = 1;
}

// Jump
def JMP_abs : I {
  let OutOperandList = (outs);
  let InOperandList = (ins brtarget:$target);
  let AsmString = "JMP $target";
  let Pattern = [(br bb:$target)];
  let Size = 3;
  let isBranch = 1; // This is a branch instruction
  let isTerminator = 1; // This instruction is part of the terminator for a basic block
  let isBarrier = 1; // Control flow cannot fall through this instruction
}

// DEX, DEY
def DE_reg : I {
  let OutOperandList = (outs General:$io);
  let InOperandList = (ins General:$ii);
  let Constraints = "$io = $ii";
  let AsmString = "DE_reg $ii";
  let Pattern = [(set General:$io, (sub General:$ii, 1))];
  let Size = 1;
}

// INX, INY
def IN_reg : I {
  let OutOperandList = (outs General:$io);
  let InOperandList = (ins General:$ii);
  let Constraints = "$io = $ii";
  let AsmString = "IN_reg $ii";
  let Pattern = [(set General:$io, (add General:$ii, 1))];
  let Size = 1;
}

// LDA, LDX, LDY
def LD_imm : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins i8imm:$x);
  let AsmString = "LD_imm $r, $x";
  let Pattern = [(set General:$r, (i8 imm:$x))];
  let Size = 2;
}

// LDA, LDX, LDY absolute address
def LD_abs : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins iPTRimm:$addr);
  let AsmString = "LD_abs $r, $addr";
  let Pattern = [(set General:$r, (load imm:$addr))];
  let Size = 3;
  let mayLoad = 1;
  let canFoldAsLoad = 1;
}

def LD_stack : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins iPTRimm:$slot, iPTRimm:$offs);
  let AsmString = "LD_stack $r, [$slot]+$offs";
  let Pattern = [(set General:$r, (load (add AddrFI:$slot, imm:$offs)))];
  let mayLoad = 1;
  let canFoldAsLoad = 1;
}

def : Pat<(i8 (load AddrFI:$slot)), (LD_stack $slot, 0)>;

// Extract Hi component from pointer register
def LD_ptr_hi : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins Ptr:$p);
  let AsmString = "LD_ptr_hi $r, $p";
  let Pattern = [(set General:$r, (M6502ptrhi Ptr:$p))];
}

// Extract Lo component from pointer register
def LD_ptr_lo : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins Ptr:$p);
  let AsmString = "LD_ptr_lo $r, $p";
  let Pattern = [(set General:$r, (M6502ptrlo Ptr:$p))];
}

// Build pointer register from Hi, Lo components
def LD_ptr_build : I {
  let OutOperandList = (outs Ptr:$p);
  let InOperandList = (ins General:$hi, General:$lo);
  let AsmString = "LD_ptr_build $p, $hi, $lo";
  let Pattern = [(set Ptr:$p, (M6502buildptr General:$hi, General:$lo))];
}

// Load pointer register from stack
def LD_ptr_stack : I {
  let OutOperandList = (outs Ptr:$p);
  let InOperandList = (ins iPTRimm:$slot, iPTRimm:$offs);
  let AsmString = "LD_ptr_stack $p, [$slot]+$offs";
  let Pattern = [(set Ptr:$p, (load (add AddrFI:$slot, imm:$offs)))];
  let mayLoad = 1;
}

def : Pat<(i16 (load AddrFI:$slot)), (LD_ptr_stack $slot, 0)>;

// Load immediate to pointer register
def LD_ptr_imm : I {
  let OutOperandList = (outs Ptr:$p);
  let InOperandList = (ins iPTRimm:$x);
  let AsmString = "LD_ptr_imm $p, $x";
  let Pattern = [(set Ptr:$p, (i16 imm:$x))];
}

// Load from memory addressed by pointer register
def LD_from_ptr : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins Ptr:$p, iPTRimm:$offs);
  let AsmString = "LD_from_ptr $r, $p+$offs";
  let Pattern = [(set General:$r, (load (add Ptr:$p, imm:$offs)))];
}

def : Pat<(i8 (load Ptr:$p)), (LD_from_ptr $p, 0)>;

// Store to memory addressed by pointer register
def ST_to_ptr : I {
  let OutOperandList = (outs);
  let InOperandList = (ins General:$r, Ptr:$p, iPTRimm:$offs);
  let AsmString = "ST_to_ptr $r, $p+$offs";
  let Pattern = [(store General:$r, (add Ptr:$p, imm:$offs))];
  let mayStore = 1;
}

def : Pat<(store General:$r, Ptr:$p), (ST_to_ptr $r, $p, 0)>;

// Store pointer to memory addressed by pointer register
// TODO: remove this instruction and split it into two general-reg stores?
def ST_ptr_to_ptr : I {
  let OutOperandList = (outs);
  let InOperandList = (ins Ptr:$r, Ptr:$p, iPTRimm:$offs);
  let AsmString = "ST_ptr_to_ptr $r, $p+$offs";
  let Pattern = [(store Ptr:$r, (add Ptr:$p, imm:$offs))];
  let mayStore = 1;
}

def : Pat<(store Ptr:$r, Ptr:$p), (ST_ptr_to_ptr $r, $p, 0)>;

def LSR_reg : I {
  let OutOperandList = (outs General:$ao);
  let InOperandList = (ins General:$ai);
  let AsmString = "LSR $ai";
  let Pattern = [(set General:$ao, (srl General:$ai, (i8 1)))];
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
def ST_abs : I {
  let OutOperandList = (outs);
  let InOperandList = (ins General:$r, iPTRimm:$addr);
  let AsmString = "ST_abs $r, $addr";
  let Pattern = [(store General:$r, imm:$addr)];
  let Size = 3;
  let mayStore = 1;
}

def ST_stack : I {
  let OutOperandList = (outs);
  let InOperandList = (ins General:$r, iPTRimm:$slot, iPTRimm:$offs);
  let AsmString = "ST_stack $r, [$slot]+$offs";
  let Pattern = [(store General:$r, (add AddrFI:$slot, imm:$offs))];
  let mayStore = 1;
}

def : Pat<(store General:$r, AddrFI:$slot), (ST_stack $r, $slot, 0)>;

// TAX, TAY, TXA, TYA (may compile as memory loads/stores)
def T_reg : I {
  let OutOperandList = (outs General:$a);
  let InOperandList = (ins General:$i);
  let AsmString = "T_reg $i, $a";
  let Pattern = []; // NOTE: instruction does not match patterns; it is
                    // generated by copyPhysReg.
  let Size = 1;
}
