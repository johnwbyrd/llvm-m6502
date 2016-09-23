// TODO: header stuff

// Custom operand types

// XXX: this seems to be missing from Target.td
let OperandType = "OPERAND_IMMEDIATE" in {
  def iPTRimm : Operand<i16>;
}

// M6502fi: Custom frameindex operand type.
// Basically, this is a hack to sneak frame-indexes past LLVM's type legalizer.
// Without this hack, LLVM tries to integer-expand FrameIndex nodes, and then
// crashes because it does not know how.
// As an added benefit, M6502fi's contain a byte offset within the stack
// variable.
// A similar hack is done for GlobalAddresses.
// FIXME: is there a better solution? Converting FrameIndexes to
// TargetFrameIndexes did not seem to work.
// TODO: An M6502fi should probably have a reference to the type of
// stack frame (i.e. a stack frame for this function, a stack frame for that
// function, etc.) This will help the late compiler pass generate native 6502
// assembly.
let OperandNamespace = "M6502" in {

let OperandType = "OPERAND_M6502_GLOBAL_ADDRESS" in
def gaoperand : Operand<OtherVT>;

let OperandType = "OPERAND_M6502_FRAME_INDEX" in
def fioperand : Operand<OtherVT>;

} // OperandNamespace = "M6502"

// TODO: move into M6502 namespace and use custom OperandType like M6502fi
def brtarget : Operand<OtherVT>;

// Custom type profiles

def SDT_M6502Wrapper : SDTypeProfile<1, 1, [SDTCisSameAs<0, 1>,
                                            SDTCisPtrTy<0>]>;
def SDT_M6502PtrElement : SDTypeProfile<1, 1, [SDTCisVT<0, i8>,
                                              SDTCisVT<1, OtherVT>]>;
def SDT_M6502LoadGA : SDTypeProfile<1, 1, [SDTCisVT<0, i8>,
                                           SDTCisVT<1, OtherVT>]>;
def SDT_M6502LoadFI : SDTypeProfile<1, 1, [SDTCisVT<0, i8>,
                                           SDTCisVT<1, OtherVT>]>;
def SDT_M6502StoreFI : SDTypeProfile<0, 2, [SDTCisVT<0, i8>,
                                            SDTCisVT<1, OtherVT>]>;
def SDT_M6502FIAddr : SDTypeProfile<1, 1, [SDTCisVT<0, i16>,
                                           SDTCisVT<1, OtherVT>]>;
def SDT_M6502Call : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_M6502Cmp : SDTypeProfile<0, 2, [SDTCisVT<0, i8>, SDTCisVT<1, i8>]>;
// FIXME: in SDT_M6502BSet, is first parameter correct for a Flag register?
def SDT_M6502BSet : SDTypeProfile<0, 2,
                                  [SDTCisInt<0>, SDTCisVT<1, OtherVT>]>;
// FIXME: in SDT_M6502BClear, is first parameter correct for a Flag register?
def SDT_M6502BClear : SDTypeProfile<0, 2,
                                    [SDTCisInt<0>, SDTCisVT<1, OtherVT>]>;
// NOTE: i16's are not allowed. Pointers are constructed from two i8's.
def SDT_M6502LoadFrom : SDTypeProfile<1, 2,
                                      [SDTCisVT<0, i8>,
                                       SDTCisVT<1, i8>, SDTCisVT<2, i8>]>;
def SDT_M6502StoreTo : SDTypeProfile<0, 3,
                                     [SDTCisVT<0, i8>,
                                      SDTCisVT<1, i8>, SDTCisVT<2, i8>]>;

// Custom nodes

// Wrapper to support converting tglobaladdr, texternalsym and tblockaddr to i16
// This solution is similar to MSP430 backend, see MSP430InstrInfo.td
def M6502wrapper : SDNode<"M6502ISD::WRAPPER", SDT_M6502Wrapper>;

// GlobalAddress-related nodes
// See FrameIndex stuff below
def M6502ga : SDNode<"M6502ISD::GA", SDTOther, []>; // Custom globaladdress node
def M6502gahi : SDNode<"M6502ISD::GAHI", SDT_M6502PtrElement, []>;
                       // Hi element of M6502ga
def M6502galo : SDNode<"M6502ISD::GALO", SDT_M6502PtrElement, []>;
                       // Lo element of M6502ga
def M6502loadga : SDNode<"M6502ISD::LOADGA", SDT_M6502LoadGA,
                         [SDNPHasChain, SDNPMayLoad]>; // load from M6502ga

// FrameIndex-related nodes
// This is a hack to work around the lack of support for:
//   (load M6502fi:$fi).
// Instead, we use (M6502loadfi M6502fi:$fi).
// FIXME: Isn't there a way to support (load M6502fi:$fi)?
def M6502fi : SDNode<"M6502ISD::FI", SDTOther, []>; // Custom frameindex node
//def M6502fi : SDNode<"M6502ISD::FI", SDT_Other, [], "M6502FINode">; // TODO
def M6502fihi : SDNode<"M6502ISD::FIHI", SDT_M6502PtrElement, []>;
                       // Hi element of M6502fi
def M6502filo : SDNode<"M6502ISD::FILO", SDT_M6502PtrElement, []>;
                       // Lo element of M6502fi
def M6502loadfi : SDNode<"M6502ISD::LOADFI", SDT_M6502LoadFI,
                         [SDNPHasChain, SDNPMayLoad]>; // load from M6502fi
def M6502storefi : SDNode<"M6502ISD::STOREFI", SDT_M6502StoreFI,
                          [SDNPHasChain, SDNPMayStore]>; // store to M6502fi
def M6502fiaddr : SDNode<"M6502ISD::FIADDR", SDT_M6502FIAddr,
                         []>; // get address of M6502fi

// Call-related nodes
def M6502call : SDNode<"M6502ISD::CALL", SDT_M6502Call,
                       [SDNPHasChain, SDNPOutGlue, SDNPInGlue, SDNPVariadic]>;
def M6502return : SDNode<"M6502ISD::RETURN", SDTNone,
                         [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;

// Comparison and conditional branching nodes
def M6502cmp : SDNode<"M6502ISD::CMP", SDT_M6502Cmp, [SDNPOutGlue]>;
def M6502bset : SDNode<"M6502ISD::BSET", SDT_M6502BSet,
                       [SDNPHasChain, SDNPInGlue, SDNPOutGlue]>;
def M6502bclear : SDNode<"M6502ISD::BCLEAR", SDT_M6502BClear,
                         [SDNPHasChain, SDNPInGlue, SDNPOutGlue]>;

// Pointer-related nodes
def M6502loadfrom : SDNode<"M6502ISD::LOADFROM", SDT_M6502LoadFrom,
                           [SDNPHasChain, SDNPMayLoad]>; // load from pointer
def M6502storeto : SDNode<"M6502ISD::STORETO", SDT_M6502StoreTo,
                          [SDNPHasChain, SDNPMayStore]>; // store to pointer

// Instructions

class I : Instruction {
  let Namespace = "M6502";
  let isPseudo = 1; // XXX: for now, all instructions are pseudo-instructions.
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

  // Stack operand
  def _stack : I {
    let OutOperandList = (outs General:$ao);
    let InOperandList = (ins General:$ai, fioperand:$fi);
    let Constraints = "$ao = $ai";
    let AsmString = AsmName # "_stack $ai, $fi";
    // TODO: stack loading instr pattern
    let Pattern = [(set General:$ao, (Oper General:$ai, (M6502loadfi M6502fi:$fi)))];
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

  // Global address
  // TODO: try to make this pattern-match with Absolute addressing instruction?
  def _ga : I {
    let OutOperandList = (outs General:$ao);
    let InOperandList = (ins General:$ai, gaoperand:$ga);
    let Constraints = "$ao = $ai";
    let AsmString = AsmName # "_abs $ai, $ga";
    let Pattern = [(set General:$ao, (Oper General:$ai, (M6502loadga M6502ga:$ga)))];
    let Size = 3;
    let mayLoad = 1;
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
  let InOperandList = (ins General:$ai, fioperand:$fi);
  //let Constraints = "";
  let AsmString = "CMP_stack $ai, $fi";
  let Pattern = [(M6502cmp General:$ai, (M6502loadfi M6502fi:$fi))];
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
  let InOperandList = (ins fioperand:$fi);
  let AsmString = "LD_stack $r, $fi";
  let Pattern = [(set General:$r, (M6502loadfi M6502fi:$fi))];
  let mayLoad = 1;
  let canFoldAsLoad = 1;
}

// Get hi byte of address of stack variable
def LD_stack_addr_hi : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins fioperand:$fi);
  let AsmString = "LD_stack_addr_hi $r, $fi";
  let Pattern = [(set General:$r, (M6502fihi M6502fi:$fi))];
}

// Get lo byte of address of stack variable
def LD_stack_addr_lo : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins fioperand:$fi);
  let AsmString = "LD_stack_addr_lo $r, $fi";
  let Pattern = [(set General:$r, (M6502filo M6502fi:$fi))];
}

// Patterns to load tglobaladdr, texternalsym and tblockaddress as immediate
// pointers.
//def : Pat<(i16 (M6502wrapper tglobaladdr:$dst)), (LD_ptr_imm tglobaladdr:$dst)>;
//def : Pat<(i16 (M6502wrapper texternalsym:$dst)), (LD_ptr_imm texternalsym:$dst)>;
//def : Pat<(i16 (M6502wrapper tblockaddress:$dst)), (LD_ptr_imm tblockaddress:$dst)>;

// Load from pointer specified by hi, lo bytes
def LD_from_ptr : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins General:$hi, General:$lo);
  let AsmString = "LD_from_ptr $r, ($hi | $lo)";
  let Pattern = [(set General:$r, (M6502loadfrom General:$hi, General:$lo))];
  let mayLoad = 1;
}

// Store to pointer specified by hi, lo bytes
def ST_to_ptr : I {
  let OutOperandList = (outs);
  let InOperandList = (ins General:$r, General:$hi, General:$lo);
  let AsmString = "ST_to_ptr $r, ($hi | $lo)";
  let Pattern = [(M6502storeto General:$r, General:$hi, General:$lo)];
  let mayStore = 1;
}

def LSR_reg : I {
  let OutOperandList = (outs General:$ao);
  let InOperandList = (ins General:$ai);
  let AsmString = "LSR $ai";
  let Pattern = [(set General:$ao, (srl General:$ai, (i8 1)))];
  let Size = 1;
}

// Generic call instruction
// The actual instructions generated depend on the calling convention. (TODO)
def CALL_imm : I {
  let isCall = 1; // This is a call instruction
  let OutOperandList = (outs);
  let InOperandList = (ins iPTRimm:$target);
  let AsmString = "CALL $target";
  let Pattern = [(M6502call imm:$target)];
}

// calls
def : Pat<(M6502call (i16 tglobaladdr:$dst)),
          (CALL_imm tglobaladdr:$dst)>;
def : Pat<(M6502call (i16 texternalsym:$dst)),
          (CALL_imm texternalsym:$dst)>;

// Generic return instruction
// This should emit RTS or JMP depending on the calling convention. (TODO)
def RETURN : I {
  let isReturn = 1; // This is a return instruction
  let isTerminator = 1; // This instruction is part of the terminator for a basic block
  let isBarrier = 1; // Control flow cannot fall through this instruction
  let OutOperandList = (outs);
  let InOperandList = (ins);
  let Pattern = [(M6502return)];
  let AsmString = "RETURN";
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
  let InOperandList = (ins General:$r, fioperand:$fi);
  let AsmString = "ST_stack $r, $fi";
  let Pattern = [(M6502storefi General:$r, M6502fi:$fi)];
  let mayStore = 1;
}

// TAX, TAY, TXA, TYA (may compile as memory loads/stores)
def T_reg : I {
  let OutOperandList = (outs General:$a);
  let InOperandList = (ins General:$i);
  let AsmString = "T_reg $i, $a";
  let Pattern = []; // NOTE: instruction does not match patterns; it is
                    // generated by copyPhysReg.
  let Size = 1;
}
