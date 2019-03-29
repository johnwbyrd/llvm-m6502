// TODO: header stuff

#include "M6502.h"
#include "M6502TargetMachine.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
using namespace llvm;

#define DEBUG_TYPE "m6502-isel"

namespace {

class M6502DAGToDAGISel : public SelectionDAGISel {
public:
  M6502DAGToDAGISel(M6502TargetMachine &TM, CodeGenOpt::Level OptLevel)
    : SelectionDAGISel(TM, OptLevel) {}
  
  bool SelectAddrFI(SDValue& N, SDValue &R);
  void Select(SDNode *N) override;

  StringRef getPassName() const override {
    return "M6502 DAG->DAG Pattern Instruction Selection";
  }

private:
  #include "M6502GenDAGISel.inc"
};

} // end anonymous namespace

FunctionPass *llvm::createM6502ISelDag(M6502TargetMachine &TM,
                                       CodeGenOpt::Level OptLevel) {
  return new M6502DAGToDAGISel(TM, OptLevel);
}

void M6502DAGToDAGISel::Select(SDNode *Node) {
  // XXX: borrowed from MipsISelDAGToDAG.cpp

  // Dump information about the Node being selected
  DEBUG(errs() << "Selecting: "; Node->dump(CurDAG); errs() << "\n");

  // If we have a custom node, we already have selected!
  if (Node->isMachineOpcode()) {
    DEBUG(errs() << "== "; Node->dump(CurDAG); errs() << "\n");
    Node->setNodeId(-1);
    return;
  }
  
  // Select the default instruction
  SelectCode(Node);
}
