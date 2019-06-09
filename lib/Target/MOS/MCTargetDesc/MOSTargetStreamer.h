//===-- MOSTargetStreamer.h - MOS Target Streamer ----------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MOS_MCTARGETDESC_MOSTARGETSTREAMER_H
#define LLVM_LIB_TARGET_MOS_MCTARGETDESC_MOSTARGETSTREAMER_H

#include "llvm/MC/MCELFStreamer.h"
#include "llvm/MC/MCStreamer.h"

namespace llvm {
class MOSTargetStreamer : public MCTargetStreamer {
  virtual void anchor();

public:
  MOSTargetStreamer(MCStreamer &S);
  /// Emit ".register <reg>, #ignore".
  virtual void emitMOSRegisterIgnore(unsigned reg) = 0;
  /// Emit ".register <reg>, #scratch".
  virtual void emitMOSRegisterScratch(unsigned reg) = 0;
};

// This part is for ascii assembly output
class MOSTargetAsmStreamer : public MOSTargetStreamer {
  formatted_raw_ostream &OS;

public:
  MOSTargetAsmStreamer(MCStreamer &S, formatted_raw_ostream &OS);
  void emitMOSRegisterIgnore(unsigned reg) override;
  void emitMOSRegisterScratch(unsigned reg) override;
};

// This part is for ELF object output
class MOSTargetELFStreamer : public MOSTargetStreamer {
public:
  MOSTargetELFStreamer(MCStreamer &S);
  MCELFStreamer &getStreamer();
  void emitMOSRegisterIgnore(unsigned reg) override {}
  void emitMOSRegisterScratch(unsigned reg) override {}
};
} // end namespace llvm

#endif
