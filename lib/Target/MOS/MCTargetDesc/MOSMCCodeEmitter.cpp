//===-- MOSMCCodeEmitter.cpp - Convert MOS code to machine code -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file implements the MOSMCCodeEmitter class.
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/MOSFixupKinds.h"
#include "MOSMCExpr.h"
#include "MOSMCTargetDesc.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCCodeEmitter.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCFixup.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/MC/SubtargetFeature.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/EndianStream.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
#include <cassert>
#include <cstdint>

using namespace llvm;

#define DEBUG_TYPE "mccodeemitter"

STATISTIC(MCNumEmitted, "Number of MC instructions emitted");

namespace {

class MOSMCCodeEmitter : public MCCodeEmitter {
  const MCInstrInfo &MCII;
  MCContext &Ctx;

public:
  MOSMCCodeEmitter(const MCInstrInfo &mcii, MCContext &ctx)
      : MCII(mcii), Ctx(ctx) {}
  MOSMCCodeEmitter(const MOSMCCodeEmitter &) = delete;
  MOSMCCodeEmitter &operator=(const MOSMCCodeEmitter &) = delete;
  ~MOSMCCodeEmitter() override = default;

  void encodeInstruction(const MCInst &MI, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

  // getBinaryCodeForInstr - TableGen'erated function for getting the
  // binary encoding for an instruction.
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  /// getMachineOpValue - Return binary encoding of operand. If the machine
  /// operand requires relocation, record the relocation and return zero.
  unsigned getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;

  unsigned getCallTargetOpValue(const MCInst &MI, unsigned OpNo,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;
  unsigned getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;
  unsigned getBranchPredTargetOpValue(const MCInst &MI, unsigned OpNo,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const;
  unsigned getBranchOnRegTargetOpValue(const MCInst &MI, unsigned OpNo,
                                       SmallVectorImpl<MCFixup> &Fixups,
                                       const MCSubtargetInfo &STI) const;

private:
  FeatureBitset computeAvailableFeatures(const FeatureBitset &FB) const;
  void
  verifyInstructionPredicates(const MCInst &MI,
                              const FeatureBitset &AvailableFeatures) const;
};

} // end anonymous namespace

void MOSMCCodeEmitter::encodeInstruction(const MCInst &MI, raw_ostream &OS,
                                           SmallVectorImpl<MCFixup> &Fixups,
                                           const MCSubtargetInfo &STI) const {
  verifyInstructionPredicates(MI,
                              computeAvailableFeatures(STI.getFeatureBits()));

  unsigned Bits = getBinaryCodeForInstr(MI, Fixups, STI);
  support::endian::write(OS, Bits,
                         Ctx.getAsmInfo()->isLittleEndian() ? support::little
                                                            : support::big);
  unsigned tlsOpNo = 0;
  switch (MI.getOpcode()) {
  default: break;
  case SP::TLS_CALL:   tlsOpNo = 1; break;
  case SP::TLS_ADDrr:
  case SP::TLS_ADDXrr:
  case SP::TLS_LDrr:
  case SP::TLS_LDXrr:  tlsOpNo = 3; break;
  }
  if (tlsOpNo != 0) {
    const MCOperand &MO = MI.getOperand(tlsOpNo);
    uint64_t op = getMachineOpValue(MI, MO, Fixups, STI);
    assert(op == 0 && "Unexpected operand value!");
    (void)op; // suppress warning.
  }

  ++MCNumEmitted;  // Keep track of the # of mi's emitted.
}

unsigned MOSMCCodeEmitter::
getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                  SmallVectorImpl<MCFixup> &Fixups,
                  const MCSubtargetInfo &STI) const {
  if (MO.isReg())
    return Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  if (MO.isImm())
    return MO.getImm();

  assert(MO.isExpr());
  const MCExpr *Expr = MO.getExpr();
  if (const MOSMCExpr *SExpr = dyn_cast<MOSMCExpr>(Expr)) {
    MCFixupKind Kind = (MCFixupKind)SExpr->getFixupKind();
    Fixups.push_back(MCFixup::create(0, Expr, Kind));
    return 0;
  }

  int64_t Res;
  if (Expr->evaluateAsAbsolute(Res))
    return Res;

  llvm_unreachable("Unhandled expression!");
  return 0;
}

unsigned MOSMCCodeEmitter::
getCallTargetOpValue(const MCInst &MI, unsigned OpNo,
                     SmallVectorImpl<MCFixup> &Fixups,
                     const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isReg() || MO.isImm())
    return getMachineOpValue(MI, MO, Fixups, STI);

  if (MI.getOpcode() == SP::TLS_CALL) {
    // No fixups for __tls_get_addr. Will emit for fixups for tls_symbol in
    // encodeInstruction.
#ifndef NDEBUG
    // Verify that the callee is actually __tls_get_addr.
    const MOSMCExpr *SExpr = dyn_cast<MOSMCExpr>(MO.getExpr());
    assert(SExpr && SExpr->getSubExpr()->getKind() == MCExpr::SymbolRef &&
           "Unexpected expression in TLS_CALL");
    const MCSymbolRefExpr *SymExpr = cast<MCSymbolRefExpr>(SExpr->getSubExpr());
    assert(SymExpr->getSymbol().getName() == "__tls_get_addr" &&
           "Unexpected function for TLS_CALL");
#endif
    return 0;
  }

  MCFixupKind fixupKind = (MCFixupKind)MOS::fixup_mos_call30;

  if (const MOSMCExpr *SExpr = dyn_cast<MOSMCExpr>(MO.getExpr())) {
    if (SExpr->getKind() == MOSMCExpr::VK_MOS_WPLT30)
      fixupKind = (MCFixupKind)MOS::fixup_mos_wplt30;
  }

  Fixups.push_back(MCFixup::create(0, MO.getExpr(), fixupKind));

  return 0;
}

unsigned MOSMCCodeEmitter::
getBranchTargetOpValue(const MCInst &MI, unsigned OpNo,
                  SmallVectorImpl<MCFixup> &Fixups,
                  const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isReg() || MO.isImm())
    return getMachineOpValue(MI, MO, Fixups, STI);

  Fixups.push_back(MCFixup::create(0, MO.getExpr(),
                                   (MCFixupKind)MOS::fixup_mos_br22));
  return 0;
}

unsigned MOSMCCodeEmitter::
getBranchPredTargetOpValue(const MCInst &MI, unsigned OpNo,
                           SmallVectorImpl<MCFixup> &Fixups,
                           const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isReg() || MO.isImm())
    return getMachineOpValue(MI, MO, Fixups, STI);

  Fixups.push_back(MCFixup::create(0, MO.getExpr(),
                                   (MCFixupKind)MOS::fixup_mos_br19));
  return 0;
}

unsigned MOSMCCodeEmitter::
getBranchOnRegTargetOpValue(const MCInst &MI, unsigned OpNo,
                           SmallVectorImpl<MCFixup> &Fixups,
                           const MCSubtargetInfo &STI) const {
  const MCOperand &MO = MI.getOperand(OpNo);
  if (MO.isReg() || MO.isImm())
    return getMachineOpValue(MI, MO, Fixups, STI);

  Fixups.push_back(MCFixup::create(0, MO.getExpr(),
                                   (MCFixupKind)MOS::fixup_mos_br16_2));
  Fixups.push_back(MCFixup::create(0, MO.getExpr(),
                                   (MCFixupKind)MOS::fixup_mos_br16_14));

  return 0;
}

#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "MOSGenMCCodeEmitter.inc"

MCCodeEmitter *llvm::createMOSMCCodeEmitter(const MCInstrInfo &MCII,
                                              const MCRegisterInfo &MRI,
                                              MCContext &Ctx) {
  return new MOSMCCodeEmitter(MCII, Ctx);
}
