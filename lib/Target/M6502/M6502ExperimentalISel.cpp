// TODO: header stuff

#include "ActionDAG.h"
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

    ActionDAG DAG;
    buildActionDAG(DAG, BB);

    //ViewGraph(&Graph, "actions." + MF.getName() + "." + BB->getName());
    // XXX: ViewGraph is broken...
    // XXX: dump all effects graphs to a dot file
    std::error_code EC;
    raw_fd_ostream DotO((Twine("actions/") + MF.getName() + "." + BB->getName() + ".dot").str(),
                        EC,
                        sys::fs::OpenFlags::F_Text);
    if (!EC) {
      WriteGraph(DotO, &DAG);
    } else {
      DEBUG(dbgs() << "Warning: failed to open file for graph output.\n"
                      "Please create a folder named `actions`.\n");
    }

    DEBUG(dbgs() << "---------- Finished analyzing basic block "
                 << BB->getName() << '\n');
  }

  return false;
}
