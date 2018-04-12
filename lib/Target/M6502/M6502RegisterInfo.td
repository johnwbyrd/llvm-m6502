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
def P0 : R<"P0">;
def P1 : R<"P1">;
def P2 : R<"P2">;
def P3 : R<"P3">;

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
def PtrReg : RegisterClass<"M6502", [i16], 16, (add P0, P1, P2, P3)>;
