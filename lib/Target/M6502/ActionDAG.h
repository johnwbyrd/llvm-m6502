// TODO: header stuff

#ifndef LLVM_LIB_TARGET_M6502_ACTIONDAG_H
#define LLVM_LIB_TARGET_M6502_ACTIONDAG_H

#include "llvm/ADT/GraphTraits.h"
#include "llvm/ADT/ilist.h"
#include "llvm/ADT/ilist_node.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/GraphWriter.h"

namespace llvm {

class BasicBlock;
class Instruction;

enum class ADNodeType {
  Invalid,
  Entry,
  Instruction,
};

/// An ActionDAG node. This represents an Action in a basic block.
class ADNode : public ilist_node<ADNode> {
  // Allow ilist_sentinel_traits to call default constructor.
  friend struct ilist_sentinel_traits<ADNode>;
private:
  ADNodeType Type = ADNodeType::Invalid;

protected:
  typedef SmallVector<ADNode *, 8> DepsVector;

  DepsVector Deps;

  ADNode() = default;
  ADNode(ADNodeType Type) : Type(Type) {}

public:
  virtual ~ADNode() {}

  typedef DepsVector::iterator deps_iterator;
  typedef DepsVector::const_iterator deps_const_iterator;

  deps_iterator deps_begin() { return Deps.begin(); }
  deps_iterator deps_end() { return Deps.end(); }
  deps_const_iterator deps_begin() const { return Deps.begin(); }
  deps_const_iterator deps_end() const { return Deps.end(); }

  ADNodeType getNodeType() const { return Type; }

  // TODO: make protected?
  void addDep(ADNode *N);
};

/// An ActionDAG node that represents an entry point to a basic block.
class EntryADNode : public ADNode {
public:
  EntryADNode() : ADNode(ADNodeType::Entry) {}

  static bool classof(const ADNode *N) {
    return N->getNodeType() == ADNodeType::Entry;
  }
};

/// An ActionDAG node that represents an LLVM instruction.
class InstrADNode : public ADNode {
protected:
  const Instruction *Instr = nullptr;

public:
  InstrADNode(const Instruction *Instr) : ADNode(ADNodeType::Instruction),
                                          Instr(Instr) {}

  const Instruction *getInstr() const { return Instr; }

  static bool classof(const ADNode *N) {
    return N->getNodeType() == ADNodeType::Instruction;
  }
};

/// A dependency graph of actions in a basic block.
/// This is like a SelectionDAG, but it is designed for M6502 code generation.
class ActionDAG {
private:
  ilist<ADNode> AllNodes;
  ADNode *Root = nullptr;

  // ActionDAGs are non-copyable.
  ActionDAG(const ActionDAG&) = delete;
  void operator=(const ActionDAG&) = delete;

  void InsertNode(ADNode *N);

public:
  ActionDAG() = default;

  typedef ilist<ADNode>::const_iterator allnodes_const_iterator;
  allnodes_const_iterator allnodes_begin() const { return AllNodes.begin(); }
  allnodes_const_iterator allnodes_end() const { return AllNodes.end(); }

  const ADNode *getRoot() const { return Root; }

  void clear();

  ADNode *makeEntryADNode();
  ADNode *makeInstrADNode(const Instruction *I);
};

// For compatibility with ViewGraph
template <>
struct GraphTraits<ADNode *> {
  typedef const ADNode NodeType;
  typedef ADNode::deps_const_iterator ChildIteratorType;
  static inline NodeType *getEntryNode(NodeType *N) { return N; }
  static inline ChildIteratorType child_begin(NodeType *N) {
    return N->deps_begin();
  }
  static inline ChildIteratorType child_end(NodeType *N) {
    return N->deps_end();
  }
};

template <>
struct GraphTraits<ActionDAG *> : public GraphTraits<ADNode *> {
  typedef ActionDAG::allnodes_const_iterator nodes_iterator;
  static nodes_iterator nodes_begin(const ActionDAG *G) {
    return G->allnodes_begin();
  }
  static nodes_iterator nodes_end(const ActionDAG *G) {
    return G->allnodes_end();
  }
};

template <>
struct DOTGraphTraits<ActionDAG *> : public DefaultDOTGraphTraits {
  explicit DOTGraphTraits(bool simple = false)
      : DefaultDOTGraphTraits(simple) {}

  static std::string getGraphName(const ActionDAG *G);

  static std::string getNodeAttributes(const ADNode *N,
                                       const ActionDAG *G);

  std::string getNodeLabel(const ADNode *Node,
                           const ActionDAG *Graph);
};

void buildActionDAG(ActionDAG &DAG, const BasicBlock *BB);

} // end namespace llvm

#endif
