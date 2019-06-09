//====- MOSMCExpr.h - MOS specific MC expression classes --*- C++ -*-=====//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file describes MOS-specific MCExprs, used for modifiers like
// "%hi" or "%lo" etc.,
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MOS_MCTARGETDESC_MOSMCEXPR_H
#define LLVM_LIB_TARGET_MOS_MCTARGETDESC_MOSMCEXPR_H

#include "MOSFixupKinds.h"
#include "llvm/MC/MCExpr.h"

namespace llvm {

class StringRef;
class MOSMCExpr : public MCTargetExpr {
public:
  enum VariantKind {
    VK_MOS_None,
    VK_MOS_LO,
    VK_MOS_HI,
    VK_MOS_H44,
    VK_MOS_M44,
    VK_MOS_L44,
    VK_MOS_HH,
    VK_MOS_HM,
    VK_MOS_PC22,
    VK_MOS_PC10,
    VK_MOS_GOT22,
    VK_MOS_GOT10,
    VK_MOS_GOT13,
    VK_MOS_13,
    VK_MOS_WPLT30,
    VK_MOS_R_DISP32,
    VK_MOS_TLS_GD_HI22,
    VK_MOS_TLS_GD_LO10,
    VK_MOS_TLS_GD_ADD,
    VK_MOS_TLS_GD_CALL,
    VK_MOS_TLS_LDM_HI22,
    VK_MOS_TLS_LDM_LO10,
    VK_MOS_TLS_LDM_ADD,
    VK_MOS_TLS_LDM_CALL,
    VK_MOS_TLS_LDO_HIX22,
    VK_MOS_TLS_LDO_LOX10,
    VK_MOS_TLS_LDO_ADD,
    VK_MOS_TLS_IE_HI22,
    VK_MOS_TLS_IE_LO10,
    VK_MOS_TLS_IE_LD,
    VK_MOS_TLS_IE_LDX,
    VK_MOS_TLS_IE_ADD,
    VK_MOS_TLS_LE_HIX22,
    VK_MOS_TLS_LE_LOX10
  };

private:
  const VariantKind Kind;
  const MCExpr *Expr;

  explicit MOSMCExpr(VariantKind Kind, const MCExpr *Expr)
      : Kind(Kind), Expr(Expr) {}

public:
  /// @name Construction
  /// @{

  static const MOSMCExpr *create(VariantKind Kind, const MCExpr *Expr,
                                 MCContext &Ctx);
  /// @}
  /// @name Accessors
  /// @{

  /// getOpcode - Get the kind of this expression.
  VariantKind getKind() const { return Kind; }

  /// getSubExpr - Get the child of this expression.
  const MCExpr *getSubExpr() const { return Expr; }

  /// getFixupKind - Get the fixup kind of this expression.
  MOS::Fixups getFixupKind() const { return getFixupKind(Kind); }

  /// @}
  void printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const override;
  bool evaluateAsRelocatableImpl(MCValue &Res,
                                 const MCAsmLayout *Layout,
                                 const MCFixup *Fixup) const override;
  void visitUsedExpr(MCStreamer &Streamer) const override;
  MCFragment *findAssociatedFragment() const override {
    return getSubExpr()->findAssociatedFragment();
  }

  void fixELFSymbolsInTLSFixups(MCAssembler &Asm) const override;

  static bool classof(const MCExpr *E) {
    return E->getKind() == MCExpr::Target;
  }

  static bool classof(const MOSMCExpr *) { return true; }

  static VariantKind parseVariantKind(StringRef name);
  static bool printVariantKind(raw_ostream &OS, VariantKind Kind);
  static MOS::Fixups getFixupKind(VariantKind Kind);
};

} // end namespace llvm.

#endif
