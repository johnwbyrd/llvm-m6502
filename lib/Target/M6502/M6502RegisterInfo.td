// TODO: header stuff

class R<string n> : Register<n> {
  let Namespace = "M6502";
}

def A : R<"A">;
def X : R<"X">;
def Y : R<"Y">;

// XXX: LLVM needs this fake register so we can use 16-bit pointers in
// instructions such as LDA, ADD, etc.
// This register does not actually exist. There are no instructions for loading
// or storing it.
def XXXFake16 : R<"XXXFake16">;

def Acc : RegisterClass<"M6502", [i8], 8,
  (add A)>;

def Index : RegisterClass<"M6502", [i8], 8,
  (add X, Y)>;

def General : RegisterClass<"M6502", [i8], 8,
  (add Acc, Index)>;

def Ptr : RegisterClass<"M6502", [i16], 16,
  (add XXXFake16)>;
