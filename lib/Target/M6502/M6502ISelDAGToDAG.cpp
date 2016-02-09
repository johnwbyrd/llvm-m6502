// TODO: header stuff

#include "M6502.h"
#include "M6502TargetMachine.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
using namespace llvm;

#define DEBUG_TYPE "m6502-isel"

namespace {

class M6502DAGToDAGISel : public SelectionDAGISel {
public:
  M6502DAGToDAGISel(M6502TargetMachine &TM, CodeGenOpt::Level OptLevel)
    : SelectionDAGISel(TM, OptLevel) {}

  SDNode *Select(SDNode *N) override;

  const char *getPassName() const override {
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

SDNode *M6502DAGToDAGISel::Select(SDNode *Node) {
  // Dump information about the Node being selected
  DEBUG(errs() << "Selecting: "; Node->dump(CurDAG); errs() << "\n");

  // XXX: borrowed from MipsISelDAGToDAG.cpp

  // Select the default instruction
  SDNode *ResNode = SelectCode(Node);

  DEBUG(errs() << "=> ");
  if (ResNode == nullptr || ResNode == Node)
	  DEBUG(Node->dump(CurDAG));
  else
	  DEBUG(ResNode->dump(CurDAG));
  DEBUG(errs() << "\n");
  return ResNode;
}
