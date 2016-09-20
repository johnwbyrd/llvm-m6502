// TODO: header stuff

class R<string n> : Register<n> {
  let Namespace = "M6502";
}

// FIXME: is this necessary and used correctly? 1-bit flags are marked
// Unallocatable as M6502 has no native i1 register type.
class Unallocatable {
  bit isAllocatable = 0;
}

// XXX: Create a set of virtual 8-bit registers. A late compiler pass will
// assign these registers to A, X, Y and memory. (TODO: this pass is not
// implemented yet.)
foreach I = 0-31 in
  def R#I : R<"R"#I>;

// XXX: Create a set of virtual 16-bit registers to hold pointers. A late
// compiler pass will assign these registers to memory. (TODO: this pass is not
// implemented yet.)
foreach I = 0-31 in
  def PTR#I : R<"PTR"#I>;

def NF : R<"NF">; // negative flag
def ZF : R<"ZF">; // zero flag
def CF : R<"CF">; // carry flag
def VF : R<"VF">; // overflow flag

def General : RegisterClass<"M6502", [i8], 8,
                            (sequence "R%u", 0, 31)>;

def Ptr : RegisterClass<"M6502", [i16], 16,
                        (sequence "PTR%u", 0, 31)>;

def Flag : RegisterClass<"M6502", [i1], 1,
                         (add NF, ZF, CF, VF)>, Unallocatable;

// TODO: register class for 16-bit pointers
