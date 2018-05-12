// TODO: header stuff

class R<string n> : Register<n> {
  let Namespace = "M6502";
}

//===----------------------------------------------------------------------===//
// Registers
//===----------------------------------------------------------------------===//

foreach I = 0-15 in {
  def R#I : R<"R"#I>;
}

foreach I = 0-15 in {
  def P#I : R<"P"#I>;
}

// Flag registers. These registers serve to coordinate Uses and Defs of
// instructions that modify or accept flags, such as CMP and BEQ.
def NF : R<"NF">; // negative flag
def ZF : R<"ZF">; // zero flag
def CF : R<"CF">; // carry flag
def VF : R<"VF">; // overflow flag

//===----------------------------------------------------------------------===//
//  Register classes
//===----------------------------------------------------------------------===//

def General : RegisterClass<"M6502", [i8], 8, (sequence "R%u", 0, 15)>;
def Pointer : RegisterClass<"M6502", [i16], 8, (sequence "P%u", 0, 15)>;
