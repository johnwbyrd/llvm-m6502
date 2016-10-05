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
def M : R<"M">;
def MH : R<"MH">; // Used as hi byte of pointer
def ML : R<"ML">; // Used as lo byte of pointer

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
def MemReg : RegisterClass<"M6502", [i8], 8, (add M, MH, ML)>;
def MHReg : RegisterClass<"M6502", [i8], 8, (add MH)>;
def MLReg : RegisterClass<"M6502", [i8], 8, (add ML)>;
def General : RegisterClass<"M6502", [i8], 8, (add PhysReg, MemReg)>;
