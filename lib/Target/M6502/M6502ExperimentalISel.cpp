// TODO: header stuff

#include "M6502.h"
#include "M6502FunctionInfo.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/IR/Function.h"
#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/PostOrderIterator.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/BranchProbabilityInfo.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Analysis/EHPersonalities.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/CodeGen/FastISel.h"
#include "llvm/CodeGen/FunctionLoweringInfo.h"
#include "llvm/CodeGen/GCMetadata.h"
#include "llvm/CodeGen/GCStrategy.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/ScheduleHazardRecognizer.h"
#include "llvm/CodeGen/SchedulerRegistry.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/StackProtector.h"
#include "llvm/CodeGen/WinEHFuncInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/Target/TargetIntrinsicInfo.h"
#include "llvm/Target/TargetLowering.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetSubtargetInfo.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
using namespace llvm;

#define DEBUG_TYPE "m6502-experimental-isel"

namespace {
class M6502ExperimentalISel final : public MachineFunctionPass {
  const char *getPassName() const override {
    return "M6502 Experimental ISel";
  }

  bool runOnMachineFunction(MachineFunction &MF) override;

public:
  static char ID;
  M6502ExperimentalISel() : MachineFunctionPass(ID) {}
};
} // end anonymous namespace

char M6502ExperimentalISel::ID = 0;
FunctionPass *llvm::createM6502ExperimentalISel() {
  return new M6502ExperimentalISel();
}

namespace llvm {

struct ActionGraph;
struct ActionNode;

struct ActionNodeRef {
  ActionNodeRef(ActionGraph *Graph = nullptr, size_t Num = -1)
      : Graph(Graph), Num(Num) {}

  ActionGraph *Graph;
  size_t Num;

  operator const ActionNode*() const;
  const ActionNode &operator*() const;
  ActionNode *operator->();
  const ActionNode *operator->() const;
};

struct ActionNode {
  typedef SmallVector<ActionNodeRef, 12> DepsVector;
  typedef DepsVector::const_iterator DepsIterator;

  enum NodeType {
    InvalidTy,
    EntryTy,
    ExitTy,
    InstTy,
  };

  ActionNodeRef NodeRef;
  NodeType Type = InvalidTy;
  const Instruction *Inst = nullptr;
  DepsVector Deps;
};

struct ActionGraph {
  typedef SmallVector<ActionNode, 12> NodeVector;
  typedef NodeVector::const_iterator NodeIterator;

  const Function *Fn = nullptr;
  NodeVector Nodes;
  ActionNodeRef Root = 0;

  ActionGraph(const Function *Fn) : Fn(Fn) {}

  ActionNodeRef createNode(ActionNode::NodeType Type,
                           const Instruction *Inst = nullptr) {
    ActionNodeRef NodeRef(this, Nodes.size());
    Nodes.push_back({});
    ActionNode *Result = &Nodes.back();
    Result->NodeRef = NodeRef;
    Result->Type = Type;
    Result->Inst = Inst;
    return NodeRef;
  }
};

ActionNodeRef::operator const ActionNode*() const {
  assert(Graph);
  return &Graph->Nodes[Num];
}

const ActionNode &ActionNodeRef::operator*() const {
  assert(Graph);
  return Graph->Nodes[Num];
}

ActionNode *ActionNodeRef::operator->() {
  assert(Graph);
  return &Graph->Nodes[Num];
}

const ActionNode *ActionNodeRef::operator->() const {
  assert(Graph);
  return &Graph->Nodes[Num];
}

// For compatibility with ViewGraph
template <>
struct GraphTraits<ActionGraph *> {
  typedef const ActionNode NodeType;
  typedef ActionNode::DepsIterator ChildIteratorType;
  typedef ActionGraph::NodeIterator nodes_iterator;

  static inline const ActionNode *getEntryNode(const ActionGraph *G) {
    return &*G->Root;
  }
  static ChildIteratorType child_begin(const NodeType *N) {
    return N->Deps.begin();
  }
  static ChildIteratorType child_end(const NodeType *N) {
    return N->Deps.end();
  }
  static nodes_iterator nodes_begin(const ActionGraph *G) {
    return G->Nodes.begin();
  }
  static nodes_iterator nodes_end(const ActionGraph *G) {
    return G->Nodes.end();
  }
};

template <>
struct DOTGraphTraits<ActionGraph *> : public DefaultDOTGraphTraits {
  explicit DOTGraphTraits(bool simple = false)
      : DefaultDOTGraphTraits(simple) {}
  
  static std::string getGraphName(const ActionGraph *G) {
    return G->Fn->getName();
  }

  std::string getNodeLabel(const ActionNode *Node,
                           const ActionGraph *Graph) {
    if (Node->Inst) {
      std::string Str;
      raw_string_ostream O(Str);
      Node->Inst->print(O);
      return Str;
    } else if (Node->Type == ActionNode::EntryTy) {
      return "(Entry)";
    } else if (Node->Type == ActionNode::ExitTy) {
      return "(Exit)";
    } else {
      return "";
    }
  }
};

} // end namespace llvm

/// Return true if an instruction is Visible.
/// An instruction is Visible if it has observable effects outside of the basic
/// block that contains it.
static bool InstructionHasVisibleActions(const Instruction &I) {
  // TODO: Other instructions may have Visible actions.
  // TODO: Some loads and stores are Invisible.
  return I.isUsedOutsideOfBlock(I.getParent()) || I.mayHaveSideEffects() ||
         I.isTerminator();
}

/// Return true if two pointers may alias.
/// This performs an extremely basic alias analysis that may return false
/// positives.
static bool MayPointersAlias(const Value *P1, const Value *P2) {
  assert(P1->getType()->isPointerTy() && P2->getType()->isPointerTy());

  if (isa<GlobalValue>(P1) && isa<GlobalValue>(P2)) {
    if (P1 != P2) {
      return false;
    }
  }

  // TODO: analyze getelementptr

  // TODO: Use more sophisticated alias analysis
  return true;
}

/// Analyze a basic block and return its Action graph.
static ActionGraph CreateActionGraph(const BasicBlock *BB) {
  // Create graph of actions.
  // This is similar to the concept of Chains in the standard SelectionDAG.
  ActionGraph Graph(BB->getParent());
  ActionNodeRef Entry = Graph.createNode(ActionNode::EntryTy); // entry node

  // Track actions for individual memory locations
  ActionNodeRef MemRoot = Entry;
  SmallVector<ActionNodeRef, 12> MemEndpoints;

  for (BasicBlock::const_iterator I = BB->begin(), E = BB->end(); I != E;
        ++I) {
    if (InstructionHasVisibleActions(*I)) {
      if (I->isTerminator() || I->getOpcode() == Instruction::Call) {
        // Join all endpoints to terminator/call
        ActionNodeRef Node = Graph.createNode(ActionNode::InstTy, &*I);
        if (!MemEndpoints.empty()) {
          Node->Deps.append(MemEndpoints.begin(), MemEndpoints.end());
        } else {
          Node->Deps.push_back(MemRoot);
        }
        MemEndpoints.clear();
        MemRoot = Node;
      } else if (I->getOpcode() == Instruction::Store) {
        const StoreInst *S = cast<StoreInst>(I);
        if (!S->isUnordered()) {
          // Store is volatile, join all endpoints
          // FIXME: not all endpoints need to be joined. analyze carefully.
          ActionNodeRef Node = Graph.createNode(ActionNode::InstTy, &*I);
          if (!MemEndpoints.empty()) {
            Node->Deps.append(MemEndpoints.begin(), MemEndpoints.end());
          } else {
            Node->Deps.push_back(MemRoot);
          }
          MemEndpoints.clear();
          MemRoot = Node;
        } else {
          ActionNodeRef Node = Graph.createNode(ActionNode::InstTy, &*I);
          // Try to attach Store node to an existing memory endpoint
          bool IsNewEndpt = true;
          for (size_t i = 0; i < MemEndpoints.size(); ++i) {
            ActionNodeRef Endpt = MemEndpoints[i];
            assert(Endpt->Inst);
            if (Endpt->Inst->getOpcode() == Instruction::Store) {
              const StoreInst *OldS = cast<StoreInst>(Endpt->Inst);
              // TODO: check if OldS and S address the same location.
              //       This requires a more sophisticated comparison.
              if (MayPointersAlias(OldS->getPointerOperand(),
                                    S->getPointerOperand())) {
                IsNewEndpt = false;
                Node->Deps.push_back(Endpt);
                MemEndpoints[i] = Node;
                break;
              }
            }
          }
          // If Store node is a new endpoint, add it to the endpoint set
          if (IsNewEndpt) {
            Node->Deps.push_back(MemRoot);
            MemEndpoints.push_back(Node);
          }
        }
      } else {
        I->dump();
        llvm_unreachable("Unhandled instruction with Visible actions");
      }
    }
  }

  // Create Exit node and join all endpoints
  Graph.Root = Graph.createNode(ActionNode::ExitTy);
  if (!MemEndpoints.empty()) {
    Graph.Root->Deps.append(MemEndpoints.begin(), MemEndpoints.end());
  } else {
    Graph.Root->Deps.push_back(MemRoot);
  }
    
}

bool M6502ExperimentalISel::runOnMachineFunction(MachineFunction &MF) {
  DEBUG(dbgs() << "********** M6502 Experimental ISel **********\n"
                  "********** Function: "
               << MF.getName() << '\n');

  M6502FunctionInfo *FuncInfo = MF.getInfo<M6502FunctionInfo>();

  ReversePostOrderTraversal<const Function*> RPOT(MF.getFunction());
  for (auto I = RPOT.begin(), E = RPOT.end(); I != E; ++I) {
    const BasicBlock *BB = *I;

    DEBUG(dbgs() << "---------- Analyzing basic block " << BB->getName() << ":\n");
    BB->dump();

    ActionGraph Graph = CreateActionGraph(BB);

    //ViewGraph(&Graph, "actions." + MF.getName() + "." + BB->getName());
    // XXX: ViewGraph is broken...
    // XXX: dump all effects graphs to a dot file
    std::error_code EC;
    raw_fd_ostream DotO((Twine("actions/") + MF.getName() + "." + BB->getName() + ".dot").str(),
                        EC,
                        sys::fs::OpenFlags::F_Text);
    if (!EC) {
      WriteGraph(DotO, &Graph);
    } else {
      DEBUG(dbgs() << "Warning: failed to open file for graph output.\n"
                      "Please create a folder named `actions`.\n");
    }

    DEBUG(dbgs() << "---------- Finished analyzing basic block "
                 << BB->getName() << '\n');
  }

  return false;
}
