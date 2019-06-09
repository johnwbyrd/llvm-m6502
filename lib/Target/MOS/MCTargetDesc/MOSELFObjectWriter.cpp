//===-- MOSELFObjectWriter.cpp - MOS ELF Writer -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "MCTargetDesc/MOSFixupKinds.h"
#include "MCTargetDesc/MOSMCExpr.h"
#include "MCTargetDesc/MOSMCTargetDesc.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCELFObjectWriter.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCObjectWriter.h"
#include "llvm/MC/MCValue.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

namespace {
  class MOSELFObjectWriter : public MCELFObjectTargetWriter {
  public:
    MOSELFObjectWriter(bool Is64Bit, uint8_t OSABI)
      : MCELFObjectTargetWriter(Is64Bit, OSABI,
                                Is64Bit ?  ELF::EM_MOSV9 : ELF::EM_MOS,
                                /*HasRelocationAddend*/ true) {}

    ~MOSELFObjectWriter() override {}

  protected:
    unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                          const MCFixup &Fixup, bool IsPCRel) const override;

    bool needsRelocateWithSymbol(const MCSymbol &Sym,
                                 unsigned Type) const override;

  };
}

unsigned MOSELFObjectWriter::getRelocType(MCContext &Ctx,
                                            const MCValue &Target,
                                            const MCFixup &Fixup,
                                            bool IsPCRel) const {

  if (const MOSMCExpr *SExpr = dyn_cast<MOSMCExpr>(Fixup.getValue())) {
    if (SExpr->getKind() == MOSMCExpr::VK_MOS_R_DISP32)
      return ELF::R_MOS_DISP32;
  }

  if (IsPCRel) {
    switch((unsigned)Fixup.getKind()) {
    default:
      llvm_unreachable("Unimplemented fixup -> relocation");
    case FK_Data_1:                  return ELF::R_MOS_DISP8;
    case FK_Data_2:                  return ELF::R_MOS_DISP16;
    case FK_Data_4:                  return ELF::R_MOS_DISP32;
    case FK_Data_8:                  return ELF::R_MOS_DISP64;
    case MOS::fixup_mos_call30:  return ELF::R_MOS_WDISP30;
    case MOS::fixup_mos_br22:    return ELF::R_MOS_WDISP22;
    case MOS::fixup_mos_br19:    return ELF::R_MOS_WDISP19;
    case MOS::fixup_mos_pc22:    return ELF::R_MOS_PC22;
    case MOS::fixup_mos_pc10:    return ELF::R_MOS_PC10;
    case MOS::fixup_mos_wplt30:  return ELF::R_MOS_WPLT30;
    }
  }

  switch((unsigned)Fixup.getKind()) {
  default:
    llvm_unreachable("Unimplemented fixup -> relocation");
  case FK_Data_1:                return ELF::R_MOS_8;
  case FK_Data_2:                return ((Fixup.getOffset() % 2)
                                         ? ELF::R_MOS_UA16
                                         : ELF::R_MOS_16);
  case FK_Data_4:                return ((Fixup.getOffset() % 4)
                                         ? ELF::R_MOS_UA32
                                         : ELF::R_MOS_32);
  case FK_Data_8:                return ((Fixup.getOffset() % 8)
                                         ? ELF::R_MOS_UA64
                                         : ELF::R_MOS_64);
  case MOS::fixup_mos_13:    return ELF::R_MOS_13;
  case MOS::fixup_mos_hi22:  return ELF::R_MOS_HI22;
  case MOS::fixup_mos_lo10:  return ELF::R_MOS_LO10;
  case MOS::fixup_mos_h44:   return ELF::R_MOS_H44;
  case MOS::fixup_mos_m44:   return ELF::R_MOS_M44;
  case MOS::fixup_mos_l44:   return ELF::R_MOS_L44;
  case MOS::fixup_mos_hh:    return ELF::R_MOS_HH22;
  case MOS::fixup_mos_hm:    return ELF::R_MOS_HM10;
  case MOS::fixup_mos_got22: return ELF::R_MOS_GOT22;
  case MOS::fixup_mos_got10: return ELF::R_MOS_GOT10;
  case MOS::fixup_mos_got13: return ELF::R_MOS_GOT13;
  case MOS::fixup_mos_tls_gd_hi22:   return ELF::R_MOS_TLS_GD_HI22;
  case MOS::fixup_mos_tls_gd_lo10:   return ELF::R_MOS_TLS_GD_LO10;
  case MOS::fixup_mos_tls_gd_add:    return ELF::R_MOS_TLS_GD_ADD;
  case MOS::fixup_mos_tls_gd_call:   return ELF::R_MOS_TLS_GD_CALL;
  case MOS::fixup_mos_tls_ldm_hi22:  return ELF::R_MOS_TLS_LDM_HI22;
  case MOS::fixup_mos_tls_ldm_lo10:  return ELF::R_MOS_TLS_LDM_LO10;
  case MOS::fixup_mos_tls_ldm_add:   return ELF::R_MOS_TLS_LDM_ADD;
  case MOS::fixup_mos_tls_ldm_call:  return ELF::R_MOS_TLS_LDM_CALL;
  case MOS::fixup_mos_tls_ldo_hix22: return ELF::R_MOS_TLS_LDO_HIX22;
  case MOS::fixup_mos_tls_ldo_lox10: return ELF::R_MOS_TLS_LDO_LOX10;
  case MOS::fixup_mos_tls_ldo_add:   return ELF::R_MOS_TLS_LDO_ADD;
  case MOS::fixup_mos_tls_ie_hi22:   return ELF::R_MOS_TLS_IE_HI22;
  case MOS::fixup_mos_tls_ie_lo10:   return ELF::R_MOS_TLS_IE_LO10;
  case MOS::fixup_mos_tls_ie_ld:     return ELF::R_MOS_TLS_IE_LD;
  case MOS::fixup_mos_tls_ie_ldx:    return ELF::R_MOS_TLS_IE_LDX;
  case MOS::fixup_mos_tls_ie_add:    return ELF::R_MOS_TLS_IE_ADD;
  case MOS::fixup_mos_tls_le_hix22:  return ELF::R_MOS_TLS_LE_HIX22;
  case MOS::fixup_mos_tls_le_lox10:  return ELF::R_MOS_TLS_LE_LOX10;
  }

  return ELF::R_MOS_NONE;
}

bool MOSELFObjectWriter::needsRelocateWithSymbol(const MCSymbol &Sym,
                                                 unsigned Type) const {
  switch (Type) {
    default:
      return false;

    // All relocations that use a GOT need a symbol, not an offset, as
    // the offset of the symbol within the section is irrelevant to
    // where the GOT entry is. Don't need to list all the TLS entries,
    // as they're all marked as requiring a symbol anyways.
    case ELF::R_MOS_GOT10:
    case ELF::R_MOS_GOT13:
    case ELF::R_MOS_GOT22:
    case ELF::R_MOS_GOTDATA_HIX22:
    case ELF::R_MOS_GOTDATA_LOX10:
    case ELF::R_MOS_GOTDATA_OP_HIX22:
    case ELF::R_MOS_GOTDATA_OP_LOX10:
      return true;
  }
}

std::unique_ptr<MCObjectTargetWriter>
llvm::createMOSELFObjectWriter(bool Is64Bit, uint8_t OSABI) {
  return llvm::make_unique<MOSELFObjectWriter>(Is64Bit, OSABI);
}
