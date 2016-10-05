// TODO: header stuff

class R<string n> : Register<n> {
  let Namespace = "M6502";
}

//===----------------------------------------------------------------------===//
// Registers
//===----------------------------------------------------------------------===//

def A : R<"A">;
def X : R<"X">;
def Y : R<"Y">;

// These are fake 8-bit registers that reside in memory.
// TODO: Fake MemRegs should be eliminated in favor of stack-vars.... maybe.
foreach I = 0-31 in
  def M#I : R<"M"#I>;

// Flag registers. These registers serve to coordinate Uses and Defs of
// instructions that modify or accept flags, such as CMP and BEQ.
def NF : R<"NF">; // negative flag
def ZF : R<"ZF">; // zero flag
def CF : R<"CF">; // carry flag
def VF : R<"VF">; // overflow flag

//===----------------------------------------------------------------------===//
//  Register classes
//===----------------------------------------------------------------------===//

def Acc : RegisterClass<"M6502", [i8], 8, (add A)>;
def Index : RegisterClass<"M6502", [i8], 8, (add X, Y)>;
def PhysReg : RegisterClass<"M6502", [i8], 8, (add Acc, Index)>;
def MemReg : RegisterClass<"M6502", [i8], 8, (sequence "M%u", 0, 31)>;
def General : RegisterClass<"M6502", [i8], 8, (add PhysReg, MemReg)>;
