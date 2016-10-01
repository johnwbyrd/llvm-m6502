// TODO: header stuff

class R<string n> : Register<n> {
  let Namespace = "M6502";
}

//===----------------------------------------------------------------------===//
// Registers
//===----------------------------------------------------------------------===//

// Register classes must have at least one register, so we define one to satisfy
// TableGen. This is not a real register in 6502. The M6502 backend uses a
// custom register allocator.
// (TODO: This custom allocator is not yet implemented.)
def G : R<"G">;

// Flag registers. These registers serve to coordinate Uses and Defs of
// instructions that modify or accept flags, such as CMP and BEQ.
def NF : R<"NF">; // negative flag
def ZF : R<"ZF">; // zero flag
def CF : R<"CF">; // carry flag
def VF : R<"VF">; // overflow flag

//===----------------------------------------------------------------------===//
//  Register classes
//===----------------------------------------------------------------------===//

def General : RegisterClass<"M6502", [i8], 8, (add G)>;
