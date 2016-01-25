// TODO: header stuff

include "llvm/Target/Target.td"

////////////////////////////////////////

include "M6502RegisterInfo.td"
include "M6502CallingConv.td"
include "M6502InstrInfo.td"

////////////////////////////////////////

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic", []>;

def M6502InstrInfo : InstrInfo;

def M6502 : Target {
  let InstructionSet = M6502InstrInfo;
}
