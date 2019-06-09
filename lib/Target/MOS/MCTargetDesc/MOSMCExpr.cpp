//===-- MOSMCExpr.cpp - MOS specific MC expression classes --------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the implementation of the assembly expression modifiers
// accepted by the MOS architecture (e.g. "%hi", "%lo", ...).
//
//===----------------------------------------------------------------------===//

#include "MOSMCExpr.h"
#include "llvm/BinaryFormat/ELF.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCObjectStreamer.h"
#include "llvm/MC/MCSymbolELF.h"

using namespace llvm;

#define DEBUG_TYPE "mosmcexpr"

const MOSMCExpr*
MOSMCExpr::create(VariantKind Kind, const MCExpr *Expr,
                      MCContext &Ctx) {
    return new (Ctx) MOSMCExpr(Kind, Expr);
}

void MOSMCExpr::printImpl(raw_ostream &OS, const MCAsmInfo *MAI) const {

  bool closeParen = printVariantKind(OS, Kind);

  const MCExpr *Expr = getSubExpr();
  Expr->print(OS, MAI);

  if (closeParen)
    OS << ')';
}

bool MOSMCExpr::printVariantKind(raw_ostream &OS, VariantKind Kind)
{
  bool closeParen = true;
  switch (Kind) {
  case VK_MOS_None:     closeParen = false; break;
  case VK_MOS_LO:       OS << "%lo(";  break;
  case VK_MOS_HI:       OS << "%hi(";  break;
  case VK_MOS_H44:      OS << "%h44("; break;
  case VK_MOS_M44:      OS << "%m44("; break;
  case VK_MOS_L44:      OS << "%l44("; break;
  case VK_MOS_HH:       OS << "%hh(";  break;
  case VK_MOS_HM:       OS << "%hm(";  break;
    // FIXME: use %pc22/%pc10, if system assembler supports them.
  case VK_MOS_PC22:     OS << "%hi("; break;
  case VK_MOS_PC10:     OS << "%lo("; break;
    // FIXME: use %got22/%got10, if system assembler supports them.
  case VK_MOS_GOT22:    OS << "%hi("; break;
  case VK_MOS_GOT10:    OS << "%lo("; break;
  case VK_MOS_GOT13:    closeParen = false; break;
  case VK_MOS_13:       closeParen = false; break;
  case VK_MOS_WPLT30:   closeParen = false; break;
  case VK_MOS_R_DISP32: OS << "%r_disp32("; break;
  case VK_MOS_TLS_GD_HI22:   OS << "%tgd_hi22(";   break;
  case VK_MOS_TLS_GD_LO10:   OS << "%tgd_lo10(";   break;
  case VK_MOS_TLS_GD_ADD:    OS << "%tgd_add(";    break;
  case VK_MOS_TLS_GD_CALL:   OS << "%tgd_call(";   break;
  case VK_MOS_TLS_LDM_HI22:  OS << "%tldm_hi22(";  break;
  case VK_MOS_TLS_LDM_LO10:  OS << "%tldm_lo10(";  break;
  case VK_MOS_TLS_LDM_ADD:   OS << "%tldm_add(";   break;
  case VK_MOS_TLS_LDM_CALL:  OS << "%tldm_call(";  break;
  case VK_MOS_TLS_LDO_HIX22: OS << "%tldo_hix22("; break;
  case VK_MOS_TLS_LDO_LOX10: OS << "%tldo_lox10("; break;
  case VK_MOS_TLS_LDO_ADD:   OS << "%tldo_add(";   break;
  case VK_MOS_TLS_IE_HI22:   OS << "%tie_hi22(";   break;
  case VK_MOS_TLS_IE_LO10:   OS << "%tie_lo10(";   break;
  case VK_MOS_TLS_IE_LD:     OS << "%tie_ld(";     break;
  case VK_MOS_TLS_IE_LDX:    OS << "%tie_ldx(";    break;
  case VK_MOS_TLS_IE_ADD:    OS << "%tie_add(";    break;
  case VK_MOS_TLS_LE_HIX22:  OS << "%tle_hix22(";  break;
  case VK_MOS_TLS_LE_LOX10:  OS << "%tle_lox10(";  break;
  }
  return closeParen;
}

MOSMCExpr::VariantKind MOSMCExpr::parseVariantKind(StringRef name)
{
  return StringSwitch<MOSMCExpr::VariantKind>(name)
    .Case("lo",  VK_MOS_LO)
    .Case("hi",  VK_MOS_HI)
    .Case("h44", VK_MOS_H44)
    .Case("m44", VK_MOS_M44)
    .Case("l44", VK_MOS_L44)
    .Case("hh",  VK_MOS_HH)
    .Case("hm",  VK_MOS_HM)
    .Case("pc22",  VK_MOS_PC22)
    .Case("pc10",  VK_MOS_PC10)
    .Case("got22", VK_MOS_GOT22)
    .Case("got10", VK_MOS_GOT10)
    .Case("got13", VK_MOS_GOT13)
    .Case("r_disp32",   VK_MOS_R_DISP32)
    .Case("tgd_hi22",   VK_MOS_TLS_GD_HI22)
    .Case("tgd_lo10",   VK_MOS_TLS_GD_LO10)
    .Case("tgd_add",    VK_MOS_TLS_GD_ADD)
    .Case("tgd_call",   VK_MOS_TLS_GD_CALL)
    .Case("tldm_hi22",  VK_MOS_TLS_LDM_HI22)
    .Case("tldm_lo10",  VK_MOS_TLS_LDM_LO10)
    .Case("tldm_add",   VK_MOS_TLS_LDM_ADD)
    .Case("tldm_call",  VK_MOS_TLS_LDM_CALL)
    .Case("tldo_hix22", VK_MOS_TLS_LDO_HIX22)
    .Case("tldo_lox10", VK_MOS_TLS_LDO_LOX10)
    .Case("tldo_add",   VK_MOS_TLS_LDO_ADD)
    .Case("tie_hi22",   VK_MOS_TLS_IE_HI22)
    .Case("tie_lo10",   VK_MOS_TLS_IE_LO10)
    .Case("tie_ld",     VK_MOS_TLS_IE_LD)
    .Case("tie_ldx",    VK_MOS_TLS_IE_LDX)
    .Case("tie_add",    VK_MOS_TLS_IE_ADD)
    .Case("tle_hix22",  VK_MOS_TLS_LE_HIX22)
    .Case("tle_lox10",  VK_MOS_TLS_LE_LOX10)
    .Default(VK_MOS_None);
}

MOS::Fixups MOSMCExpr::getFixupKind(MOSMCExpr::VariantKind Kind) {
  switch (Kind) {
  default: llvm_unreachable("Unhandled MOSMCExpr::VariantKind");
  case VK_MOS_LO:       return MOS::fixup_mos_lo10;
  case VK_MOS_HI:       return MOS::fixup_mos_hi22;
  case VK_MOS_H44:      return MOS::fixup_mos_h44;
  case VK_MOS_M44:      return MOS::fixup_mos_m44;
  case VK_MOS_L44:      return MOS::fixup_mos_l44;
  case VK_MOS_HH:       return MOS::fixup_mos_hh;
  case VK_MOS_HM:       return MOS::fixup_mos_hm;
  case VK_MOS_PC22:     return MOS::fixup_mos_pc22;
  case VK_MOS_PC10:     return MOS::fixup_mos_pc10;
  case VK_MOS_GOT22:    return MOS::fixup_mos_got22;
  case VK_MOS_GOT10:    return MOS::fixup_mos_got10;
  case VK_MOS_GOT13:    return MOS::fixup_mos_got13;
  case VK_MOS_13:       return MOS::fixup_mos_13;
  case VK_MOS_WPLT30:   return MOS::fixup_mos_wplt30;
  case VK_MOS_TLS_GD_HI22:   return MOS::fixup_mos_tls_gd_hi22;
  case VK_MOS_TLS_GD_LO10:   return MOS::fixup_mos_tls_gd_lo10;
  case VK_MOS_TLS_GD_ADD:    return MOS::fixup_mos_tls_gd_add;
  case VK_MOS_TLS_GD_CALL:   return MOS::fixup_mos_tls_gd_call;
  case VK_MOS_TLS_LDM_HI22:  return MOS::fixup_mos_tls_ldm_hi22;
  case VK_MOS_TLS_LDM_LO10:  return MOS::fixup_mos_tls_ldm_lo10;
  case VK_MOS_TLS_LDM_ADD:   return MOS::fixup_mos_tls_ldm_add;
  case VK_MOS_TLS_LDM_CALL:  return MOS::fixup_mos_tls_ldm_call;
  case VK_MOS_TLS_LDO_HIX22: return MOS::fixup_mos_tls_ldo_hix22;
  case VK_MOS_TLS_LDO_LOX10: return MOS::fixup_mos_tls_ldo_lox10;
  case VK_MOS_TLS_LDO_ADD:   return MOS::fixup_mos_tls_ldo_add;
  case VK_MOS_TLS_IE_HI22:   return MOS::fixup_mos_tls_ie_hi22;
  case VK_MOS_TLS_IE_LO10:   return MOS::fixup_mos_tls_ie_lo10;
  case VK_MOS_TLS_IE_LD:     return MOS::fixup_mos_tls_ie_ld;
  case VK_MOS_TLS_IE_LDX:    return MOS::fixup_mos_tls_ie_ldx;
  case VK_MOS_TLS_IE_ADD:    return MOS::fixup_mos_tls_ie_add;
  case VK_MOS_TLS_LE_HIX22:  return MOS::fixup_mos_tls_le_hix22;
  case VK_MOS_TLS_LE_LOX10:  return MOS::fixup_mos_tls_le_lox10;
  }
}

bool
MOSMCExpr::evaluateAsRelocatableImpl(MCValue &Res,
                                       const MCAsmLayout *Layout,
                                       const MCFixup *Fixup) const {
  return getSubExpr()->evaluateAsRelocatable(Res, Layout, Fixup);
}

static void fixELFSymbolsInTLSFixupsImpl(const MCExpr *Expr, MCAssembler &Asm) {
  switch (Expr->getKind()) {
  case MCExpr::Target:
    llvm_unreachable("Can't handle nested target expr!");
    break;

  case MCExpr::Constant:
    break;

  case MCExpr::Binary: {
    const MCBinaryExpr *BE = cast<MCBinaryExpr>(Expr);
    fixELFSymbolsInTLSFixupsImpl(BE->getLHS(), Asm);
    fixELFSymbolsInTLSFixupsImpl(BE->getRHS(), Asm);
    break;
  }

  case MCExpr::SymbolRef: {
    const MCSymbolRefExpr &SymRef = *cast<MCSymbolRefExpr>(Expr);
    cast<MCSymbolELF>(SymRef.getSymbol()).setType(ELF::STT_TLS);
    break;
  }

  case MCExpr::Unary:
    fixELFSymbolsInTLSFixupsImpl(cast<MCUnaryExpr>(Expr)->getSubExpr(), Asm);
    break;
  }

}

void MOSMCExpr::fixELFSymbolsInTLSFixups(MCAssembler &Asm) const {
  switch(getKind()) {
  default: return;
  case VK_MOS_TLS_GD_CALL:
  case VK_MOS_TLS_LDM_CALL: {
    // The corresponding relocations reference __tls_get_addr, as they call it,
    // but this is only implicit; we must explicitly add it to our symbol table
    // to bind it for these uses.
    MCSymbol *Symbol = Asm.getContext().getOrCreateSymbol("__tls_get_addr");
    Asm.registerSymbol(*Symbol);
    auto ELFSymbol = cast<MCSymbolELF>(Symbol);
    if (!ELFSymbol->isBindingSet()) {
      ELFSymbol->setBinding(ELF::STB_GLOBAL);
      ELFSymbol->setExternal(true);
    }
    LLVM_FALLTHROUGH;
  }
  case VK_MOS_TLS_GD_HI22:
  case VK_MOS_TLS_GD_LO10:
  case VK_MOS_TLS_GD_ADD:
  case VK_MOS_TLS_LDM_HI22:
  case VK_MOS_TLS_LDM_LO10:
  case VK_MOS_TLS_LDM_ADD:
  case VK_MOS_TLS_LDO_HIX22:
  case VK_MOS_TLS_LDO_LOX10:
  case VK_MOS_TLS_LDO_ADD:
  case VK_MOS_TLS_IE_HI22:
  case VK_MOS_TLS_IE_LO10:
  case VK_MOS_TLS_IE_LD:
  case VK_MOS_TLS_IE_LDX:
  case VK_MOS_TLS_IE_ADD:
  case VK_MOS_TLS_LE_HIX22:
  case VK_MOS_TLS_LE_LOX10: break;
  }
  fixELFSymbolsInTLSFixupsImpl(getSubExpr(), Asm);
}

void MOSMCExpr::visitUsedExpr(MCStreamer &Streamer) const {
  Streamer.visitUsedExpr(*getSubExpr());
}
