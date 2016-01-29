// TODO: header stuff

#ifndef LLVM_LIB_TARGET_MOS6502_MOS6502TARGETOBJECTFILE_H
#define LLVM_LIB_TARGET_MOS6502_MOS6502TARGETOBJECTFILE_H

#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"

namespace llvm {

class Mos6502TargetObjectFile : public TargetLoweringObjectFile {

public:
  MCSection *getExplicitSectionGlobal(const GlobalValue *GV, SectionKind Kind,
                                      Mangler &Mang,
                                      const TargetMachine &TM) const override;

  MCSection *SelectSectionForGlobal(const GlobalValue *GV, SectionKind Kind,
                                    Mangler &Mang,
                                    const TargetMachine &TM) const override;
};

} // end namespace llvm

#endif
