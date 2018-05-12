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

// Fake pointer registers for holding the high and low bytes of an address.
def PL : R<"PL">;
def PH : R<"PH">;

// Fake memory registers for holding an operand
def M0 : R<"M0">;
def M1 : R<"M1">;
def M2 : R<"M2">;
def M3 : R<"M3">;

// XXX: Unused 16-bit register to make TableGen shut up.
def XXX1: R<"XXX1">;

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
def YReg : RegisterClass<"M6502", [i8], 8, (add Y)>;
def Index : RegisterClass<"M6502", [i8], 8, (add X, YReg)>;
def PhysReg : RegisterClass<"M6502", [i8], 8, (add Acc, Index)>;
def MemReg : RegisterClass<"M6502", [i8], 8, (add M0, M1, M2, M3)>;
def General : RegisterClass<"M6502", [i8], 8, (add PhysReg)>;
def Pointer : RegisterClass<"M6502", [i8], 8, (add PL, PH)>;
def All : RegisterClass<"M6502", [i8], 8, (add General, Pointer, MemReg)>;
def XXXReg : RegisterClass<"M6502", [i16], 16, (add XXX1)>;
