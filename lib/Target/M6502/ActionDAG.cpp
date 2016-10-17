// TODO: header stuff

#include "ActionDAG.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/Debug.h"
using namespace llvm;

#define DEBUG_TYPE "m6502-action-dag"

void ADNode::addDep(ADNode *N) {
  // TODO: make Deps a Set instead of a Vector?
  Deps.push_back(N);
}

void ActionDAG::InsertNode(ADNode *N) {
  AllNodes.push_back(N);
}

void ActionDAG::clear() {
  // TODO: destroy all nodes safely
  AllNodes.clear();
  Root = nullptr;
}

ADNode *ActionDAG::makeEntryADNode() {
  ADNode *N = new EntryADNode();
  InsertNode(N);
  return N;
}

ADNode *ActionDAG::makeInstrADNode(const Instruction *I) {
  ADNode *N = new InstrADNode(I);
  InsertNode(N);
  return N;
}

std::string DOTGraphTraits<ActionDAG *>::getGraphName(const ActionDAG *G) {
  return ""; // TODO: return function name
}

std::string DOTGraphTraits<ActionDAG *>::getNodeAttributes(const ADNode *N,
                                                         const ActionDAG *G) {
  return ""; // TODO
}

std::string DOTGraphTraits<ActionDAG *>::getNodeLabel(const ADNode *Node,
                                                    const ActionDAG *Graph) {
  if (isa<EntryADNode>(Node)) {
    return "(Entry)";
  } else if (isa<InstrADNode>(Node)) {
    const InstrADNode *INode = cast<InstrADNode>(Node);
    std::string OStr;
    raw_string_ostream O(OStr);
    INode->getInstr()->print(O);
    return OStr;
  } else {
    return "***INVALID NODE***";
  }
}

void llvm::buildActionDAG(ActionDAG &DAG, const BasicBlock *BB) {
  DAG.clear();
  
  typedef DenseMap<const Instruction *, ADNode *> InstrActionMap;
  InstrActionMap InstrActions;

  ADNode *MemChain = DAG.makeEntryADNode();
  SmallVector<ADNode *, 8> OutgoingNodes;

  for (BasicBlock::const_iterator It = BB->begin(), E = BB->end(); It != E;
       ++It) {
    const Instruction *I = &*It;

    ADNode *Node = DAG.makeInstrADNode(I);

    if (I->isUsedOutsideOfBlock(BB)) {
      OutgoingNodes.push_back(Node);
    }

    // Hook up memory dependencies to node
    if (I->mayReadFromMemory()) {
      Node->addDep(MemChain);
    }

    if (I->mayHaveSideEffects()) {
      Node->addDep(MemChain);
      MemChain = Node;
    }

    // Hook up operand dependencies to node
    for (unsigned int OpIdx = 0, NumOps = I->getNumOperands(); OpIdx < NumOps;
         ++OpIdx) {
      const Value *Operand = I->getOperand(OpIdx);
      if (isa<Instruction>(Operand)) {
        const Instruction *OperI = cast<Instruction>(Operand);
        InstrActionMap::const_iterator amit = InstrActions.find(OperI);
        if (amit != InstrActions.end()) {
          Node->addDep(amit->second);
        } else {
          I->dump();
          dbgs() << "WARNING: operand dependency not found";
        }
      }
    }

    if (I->isTerminator()) {
      // Terminate memory chain and attach outgoing nodes
      Node->addDep(MemChain);
      for (ADNode *N : OutgoingNodes) {
        Node->addDep(N);
      }
    }

    InstrActions[I] = Node;
  }
}