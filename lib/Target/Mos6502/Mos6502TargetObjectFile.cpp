// TODO: header stuff

#include "Mos6502TargetObjectFile.h"

using namespace llvm;

MCSection *
Mos6502TargetObjectFile::getExplicitSectionGlobal(const GlobalValue *GV,
                                                SectionKind Kind,
                                                Mangler &Mang,
                                                const TargetMachine &TM) const {
  // TODO
  return nullptr;
}

MCSection *
Mos6502TargetObjectFile::SelectSectionForGlobal(const GlobalValue *GV,
                                                SectionKind Kind, Mangler &Mang,
                                                const TargetMachine &TM) const {
  // TODO
  return nullptr;
}
