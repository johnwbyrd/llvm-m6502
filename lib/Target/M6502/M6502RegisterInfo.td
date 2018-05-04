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
def General : RegisterClass<"M6502", [i8], 8, (add PhysReg)>;
def Pointer : RegisterClass<"M6502", [i8], 8, (add PL, PH)>;
def XXXReg : RegisterClass<"M6502", [i16], 16, (add XXX1)>;
