// TODO: header stuff

#ifndef LLVM_LIB_TARGET_M6502_M6502SELECTIONDAGINFO_H
#define LLVM_LIB_TARGET_M6502_M6502SELECTIONDAGINFO_H

#include "llvm/Target/TargetSelectionDAGInfo.h"

namespace llvm {

class M6502SelectionDAGInfo : public TargetSelectionDAGInfo {
public:
  // TODO: XCore target has EmitTargetCodeForMemcpy
};

} // end namespace llvm

#endif
