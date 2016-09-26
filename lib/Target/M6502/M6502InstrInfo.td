// TODO: header stuff

// Custom operand types

// XXX: this seems to be missing from Target.td
let OperandType = "OPERAND_IMMEDIATE" in {
  def iPTRimm : Operand<i16>;
}

let OperandNamespace = "M6502" in {

//let OperandType = "OPERAND_M6502_GLOBAL_ADDRESS" in
//def gaoperand : Operand<OtherVT>;

// TODO: implement frameindex operand
//let OperandType = "OPERAND_M6502_FRAME_INDEX" in
//def fioperand : Operand<OtherVT>;

} // OperandNamespace = "M6502"

// TODO: move into M6502 namespace
def brtarget : Operand<OtherVT>;

// Custom type profiles

def SDT_M6502AbsAddr : SDTypeProfile<1, 1, [SDTCisVT<0, OtherVT>,
                                            SDTCisVT<1, i16>]>;
def SDT_M6502HiLoAddr : SDTypeProfile<1, 2, [SDTCisVT<0, OtherVT>,
                                             SDTCisVT<1, i8>,
                                             SDTCisVT<2, i8>]>;
def SDT_M6502FIAddr : SDTypeProfile<1, 2, [SDTCisVT<0, OtherVT>,
                                           SDTCisVT<1, i16>,
                                           SDTCisVT<2, i16>]>;
def SDT_M6502AddrElement : SDTypeProfile<1, 1, [SDTCisVT<0, i8>,
                                                SDTCisVT<1, i16>]>;
def SDT_M6502LoadFrom : SDTypeProfile<1, 1,
                                     [SDTCisVT<0, i8>,
                                      SDTCisVT<1, OtherVT>]>;
def SDT_M6502StoreTo : SDTypeProfile<0, 2,
                                    [SDTCisVT<0, i8>,
                                     SDTCisVT<1, OtherVT>]>;

def SDT_M6502Call : SDTypeProfile<0, -1, [SDTCisVT<0, iPTR>]>;
def SDT_M6502Cmp : SDTypeProfile<0, 2, [SDTCisVT<0, i8>, SDTCisVT<1, i8>]>;
// FIXME: in SDT_M6502BSet, is first parameter correct for a Flag register?
def SDT_M6502BSet : SDTypeProfile<0, 2,
                                  [SDTCisInt<0>, SDTCisVT<1, OtherVT>]>;
// FIXME: in SDT_M6502BClear, is first parameter correct for a Flag register?
def SDT_M6502BClear : SDTypeProfile<0, 2,
                                    [SDTCisInt<0>, SDTCisVT<1, OtherVT>]>;

// Custom nodes

// Address-related nodes
def M6502absaddr : SDNode<"M6502ISD::ABSADDR", SDT_M6502AbsAddr,
                          []>; // get absolute address
def M6502hiloaddr : SDNode<"M6502ISD::HILOADDR", SDT_M6502HiLoAddr,
                           []>; // get address from Hi, Lo bytes
def M6502fiaddr : SDNode<"M6502ISD::FIADDR", SDT_M6502FIAddr,
                         []>; // get address of frameindex
def M6502addrhi : SDNode<"M6502ISD::ADDRHI", SDT_M6502AddrElement, []>;
                      // Hi element of absolute address
def M6502addrlo : SDNode<"M6502ISD::ADDRLO", SDT_M6502AddrElement, []>;
                      // Lo element of absolute address
def M6502fihi : SDNode<"M6502ISD::FIHI", SDT_M6502AddrElement, []>;
                      // Hi element of frameindex address
def M6502filo : SDNode<"M6502ISD::FILO", SDT_M6502AddrElement, []>;
                      // Lo element of frameindex address

 // Loading and storing-related nodes
 def M6502loadfrom : SDNode<"M6502ISD::LOADFROM", SDT_M6502LoadFrom,
                            [SDNPHasChain, SDNPMayLoad]>; // load from pointer
 def M6502storeto : SDNode<"M6502ISD::STORETO", SDT_M6502StoreTo,
                           [SDNPHasChain, SDNPMayStore]>; // store to pointer


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

  // Immediate
  def _imm : I {
    let OutOperandList = (outs General:$ao);
    let InOperandList = (ins General:$ai, i8imm:$x);
    let Constraints = "$ao = $ai";
    let AsmString = AsmName # "_imm $ai, $x";
    let Pattern = [(set General:$ao, (Oper General:$ai, imm:$x))];
    let Size = 2;
  }

  // TODO: stack operand
  // TODO: absolute address operand
  // TODO: indexed address operand
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

// Shift left by immediate amount
// FIXME: This is not a real instruction in 6502. The closest equivalent is
// ASL, which shifts A left by one bit.
def ASL_imm : I {
  let OutOperandList = (outs General:$ao);
  let InOperandList = (ins General:$ai, i8imm:$x);
  let Constraints = "$ao = $ai";
  let AsmString = "ASL_imm $ai, $x";
  let Pattern = [(set General:$ao, (shl General:$ai, (i8 imm:$x)))];
}

// Arithmetic shift right
// FIXME: This is not a real instruction in 6502. This instruction must be
// expanded to CMP's and ROR's. For example, see
// <https://wiki.nesdev.com/w/index.php/6502_assembly_optimisations#Arithmetic_shift_right>
def ASR_imm : I {
  let OutOperandList = (outs General:$ao);
  let InOperandList = (ins General:$ai, i8imm:$x);
  let Constraints = "$ao = $ai";
  let AsmString = "ASR_imm $ai, $x";
  let Pattern = [(set General:$ao, (sra General:$ai, (i8 imm:$x)))];
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

// Compare Register with Immediate
// TODO: Define CMP with a multiclass like AccOperatorI
def CMP_imm : I {
  let Defs = [NF, ZF, CF];
  let OutOperandList = (outs);
  let InOperandList = (ins General:$ai, i8imm:$x);
  let AsmString = "CMP_imm $ai, $x";
  let Pattern = [(M6502cmp General:$ai, imm:$x)];
}

// Compare Register with Register
// TODO: Define CMP with a multiclass like AccOperatorI
def CMP_reg : I {
  let Defs = [NF, ZF, CF];
  let OutOperandList = (outs);
  let InOperandList = (ins General:$ai, General:$x);
  let AsmString = "CMP_reg $ai, $x";
  let Pattern = [(M6502cmp General:$ai, General:$x)];
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

// Get hi byte of address (tglobaladdr, texternalsym or tblockaddress)
def LD_addr_hi : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins iPTRimm:$addr);
  let AsmString = "LD_addr_hi $r, $addr";
  let Pattern = [(set General:$r, (M6502addrhi timm:$addr))];
}

// Get lo byte of address (tglobaladdr, texternalsym or tblockaddress)
def LD_addr_lo : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins iPTRimm:$addr);
  let AsmString = "LD_addr_lo $r, $addr";
  let Pattern = [(set General:$r, (M6502addrlo timm:$addr))];
}

def : Pat<(M6502addrhi tglobaladdr:$addr), (LD_addr_hi tglobaladdr:$addr)>;
def : Pat<(M6502addrlo tglobaladdr:$addr), (LD_addr_lo tglobaladdr:$addr)>;

// Get hi byte of address of stack variable
def LD_stack_addr_hi : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins iPTRimm:$fi);
  let AsmString = "LD_stack_addr_hi $r, [$fi]";
  let Pattern = [(set General:$r, (M6502fihi timm:$fi))];
}

// Get lo byte of address of stack variable
def LD_stack_addr_lo : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins iPTRimm:$fi);
  let AsmString = "LD_stack_addr_lo $r, [$fi]";
  let Pattern = [(set General:$r, (M6502filo timm:$fi))];
}

// Load from absolute address
def LD_abs : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins iPTRimm:$addr);
  let AsmString = "LD_abs $r, $addr";
  let Pattern = [(set General:$r, (M6502loadfrom (M6502absaddr timm:$addr)))];
  let mayLoad = 1;
  let canFoldAsLoad = 1;
}

def : Pat<(M6502loadfrom (M6502absaddr tglobaladdr:$addr)),
          (LD_abs tglobaladdr:$addr)>;
// TODO: texternalsym, etc.

// Load from address formed by Hi, Lo bytes
def LD_hi_lo : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins General:$hi, General:$lo);
  let AsmString = "LD_hi_lo $r, ($hi, $lo)";
  let Pattern = [(set General:$r, (M6502loadfrom (M6502hiloaddr General:$hi, General:$lo)))];
  let mayLoad = 1;
  let canFoldAsLoad = 1;
}

// Load from stack
def LD_stack : I {
  let OutOperandList = (outs General:$r);
  let InOperandList = (ins iPTRimm:$fi, iPTRimm:$offs);
  let AsmString = "LD_stack $r, [$fi]+$offs";
  let Pattern = [(set General:$r, (M6502loadfrom (M6502fiaddr timm:$fi, timm:$offs)))];
  let mayLoad = 1;
  let canFoldAsLoad = 1;
}

// Store to absolute address
def ST_abs : I {
  let OutOperandList = (outs);
  let InOperandList = (ins General:$r, iPTRimm:$addr);
  let AsmString = "ST_abs $r, $addr";
  let Pattern = [(M6502storeto General:$r, (M6502absaddr timm:$addr))];
  let mayStore = 1;
}

def : Pat<(M6502storeto General:$r, (M6502absaddr tglobaladdr:$addr)),
          (ST_abs General:$r, tglobaladdr:$addr)>;
// TODO: texternalsym, etc.

// Store to address formed by Hi, Lo bytes
def ST_hi_lo : I {
  let OutOperandList = (outs);
  let InOperandList = (ins General:$r, General:$hi, General:$lo);
  let AsmString = "ST_hi_lo $r, ($hi, $lo)";
  let Pattern = [(M6502storeto General:$r, (M6502hiloaddr General:$hi, General:$lo))];
  let mayStore = 1;
}

// Store to stack
def ST_stack : I {
  let OutOperandList = (outs);
  let InOperandList = (ins General:$r, iPTRimm:$fi, iPTRimm:$offs);
  let AsmString = "ST_stack $r, [$fi]+$offs";
  let Pattern = [(M6502storeto General:$r, (M6502fiaddr timm:$fi, timm:$offs))];
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
  let Pattern = [(M6502call timm:$target)];
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

// TAX, TAY, TXA, TYA (may compile as memory loads/stores)
def T_reg : I {
  let OutOperandList = (outs General:$dst);
  let InOperandList = (ins General:$src);
  let AsmString = "T_reg $dst, $src";
  let Pattern = [];
  // NOTE: This instruction is generated by copyPhysReg.
  // Do NOT attempt to use the pattern below! If the pattern below is
  // uncommented, LLVM will mistakenly use T_reg as a "catch-all" for patterns
  // that have no other matches, leading to output like:
  //   T_reg R0, R0
  // if no other instruction that assigns the correct value to R0 can be found.
  //let Pattern = [(set General:$dst, General:$src)];
}
