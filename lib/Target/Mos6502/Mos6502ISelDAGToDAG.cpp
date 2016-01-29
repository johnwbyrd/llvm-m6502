// TODO: header stuff

#include "Mos6502.h"
#include "Mos6502TargetMachine.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
using namespace llvm;

#define DEBUG_TYPE "mos6502-isel"

namespace {

class Mos6502DAGToDAGISel : public SelectionDAGISel {
public:
  Mos6502DAGToDAGISel(Mos6502TargetMachine &TM, CodeGenOpt::Level OptLevel)
    : SelectionDAGISel(TM, OptLevel) {}

  SDNode *Select(SDNode *N) override;

  const char *getPassName() const override {
    return "Mos6502 DAG->DAG Pattern Instruction Selection";
  }

private:
  #include "Mos6502GenDAGISel.inc"
};

} // end anonymous namespace

FunctionPass *llvm::createMos6502ISelDag(Mos6502TargetMachine &TM,
                                         CodeGenOpt::Level OptLevel) {
  return new Mos6502DAGToDAGISel(TM, OptLevel);
}

SDNode *Mos6502DAGToDAGISel::Select(SDNode *Node) {
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
