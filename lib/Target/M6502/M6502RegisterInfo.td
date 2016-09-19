// TODO: header stuff

class R<string n> : Register<n> {
  let Namespace = "M6502";
}

def A : R<"A">;
def X : R<"X">;
def Y : R<"Y">;
def NFlag : R<"NFlag">; // negative flag
def ZFlag : R<"ZFlag">; // zero flag
def CFlag : R<"CFlag">; // carry flag
def VFlag : R<"VFlag">; // overflow flag

/*
// XXX: Fake 16-bit registers for pseudo-instructions that operate on 16-bit
// pointers.
def FakePtr0 : R<"FakePtr0">;
def FakePtr1 : R<"FakePtr1">;
def FakePtr2 : R<"FakePtr2">;
def FakePtr3 : R<"FakePtr3">;
def FakePtr4 : R<"FakePtr4">;
def FakePtr5 : R<"FakePtr5">;
def FakePtr6 : R<"FakePtr6">;
def FakePtr7 : R<"FakePtr7">;
*/

def Acc : RegisterClass<"M6502", [i8], 8,
  (add A)>;

def Index : RegisterClass<"M6502", [i8], 8,
  (add X, Y)>;

def General : RegisterClass<"M6502", [i8], 8,
  (add Acc, Index)>;

def Flag : RegisterClass<"M6502", [i1], 1,
  (add NFlag, ZFlag, CFlag, VFlag)>;

/*
def Ptr : RegisterClass<"M6502", [i16], 16,
  (add FakePtr0, FakePtr1, FakePtr2, FakePtr3,
       FakePtr4, FakePtr5, FakePtr6, FakePtr7)>;
*/
