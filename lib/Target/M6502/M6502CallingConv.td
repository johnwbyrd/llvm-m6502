// TODO: header stuff

def RetCC_M6502 : CallingConv<[
  CCIfType<[i8], CCAssignToReg<[A, X, Y]>>,
  CCAssignToStack<1, 1>
]>;

def CC_M6502 : CallingConv<[
  CCIfType<[i8], CCAssignToReg<[A, X, Y]>>,
  CCAssignToStack<1, 1>
]>;
