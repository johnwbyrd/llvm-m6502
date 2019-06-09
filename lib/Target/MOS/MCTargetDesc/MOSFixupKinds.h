//===-- MOSFixupKinds.h - MOS Specific Fixup Entries --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MOS_MCTARGETDESC_MOSFIXUPKINDS_H
#define LLVM_LIB_TARGET_MOS_MCTARGETDESC_MOSFIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

namespace llvm {
  namespace MOS {
    enum Fixups {
      // fixup_mos_call30 - 30-bit PC relative relocation for call
      fixup_mos_call30 = FirstTargetFixupKind,

      /// fixup_mos_br22 - 22-bit PC relative relocation for
      /// branches
      fixup_mos_br22,

      /// fixup_mos_br19 - 19-bit PC relative relocation for
      /// branches on icc/xcc
      fixup_mos_br19,

      /// fixup_mos_bpr  - 16-bit fixup for bpr
      fixup_mos_br16_2,
      fixup_mos_br16_14,

      /// fixup_mos_13 - 13-bit fixup
      fixup_mos_13,

      /// fixup_mos_hi22  - 22-bit fixup corresponding to %hi(foo)
      /// for sethi
      fixup_mos_hi22,

      /// fixup_mos_lo10  - 10-bit fixup corresponding to %lo(foo)
      fixup_mos_lo10,

      /// fixup_mos_h44  - 22-bit fixup corresponding to %h44(foo)
      fixup_mos_h44,

      /// fixup_mos_m44  - 10-bit fixup corresponding to %m44(foo)
      fixup_mos_m44,

      /// fixup_mos_l44  - 12-bit fixup corresponding to %l44(foo)
      fixup_mos_l44,

      /// fixup_mos_hh  -  22-bit fixup corresponding to %hh(foo)
      fixup_mos_hh,

      /// fixup_mos_hm  -  10-bit fixup corresponding to %hm(foo)
      fixup_mos_hm,

      /// fixup_mos_pc22 - 22-bit fixup corresponding to %pc22(foo)
      fixup_mos_pc22,

      /// fixup_mos_pc10 - 10-bit fixup corresponding to %pc10(foo)
      fixup_mos_pc10,

      /// fixup_mos_got22 - 22-bit fixup corresponding to %got22(foo)
      fixup_mos_got22,

      /// fixup_mos_got10 - 10-bit fixup corresponding to %got10(foo)
      fixup_mos_got10,

      /// fixup_mos_got13 - 13-bit fixup corresponding to %got13(foo)
      fixup_mos_got13,

      /// fixup_mos_wplt30
      fixup_mos_wplt30,

      /// fixups for Thread Local Storage
      fixup_mos_tls_gd_hi22,
      fixup_mos_tls_gd_lo10,
      fixup_mos_tls_gd_add,
      fixup_mos_tls_gd_call,
      fixup_mos_tls_ldm_hi22,
      fixup_mos_tls_ldm_lo10,
      fixup_mos_tls_ldm_add,
      fixup_mos_tls_ldm_call,
      fixup_mos_tls_ldo_hix22,
      fixup_mos_tls_ldo_lox10,
      fixup_mos_tls_ldo_add,
      fixup_mos_tls_ie_hi22,
      fixup_mos_tls_ie_lo10,
      fixup_mos_tls_ie_ld,
      fixup_mos_tls_ie_ldx,
      fixup_mos_tls_ie_add,
      fixup_mos_tls_le_hix22,
      fixup_mos_tls_le_lox10,

      // Marker
      LastTargetFixupKind,
      NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
    };
  }
}

#endif
