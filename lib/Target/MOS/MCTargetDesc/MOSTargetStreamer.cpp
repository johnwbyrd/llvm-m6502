//===-- MOSTargetStreamer.cpp - MOS Target Streamer Methods -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides MOS specific target streamer methods.
//
//===----------------------------------------------------------------------===//

#include "MOSTargetStreamer.h"
#include "MOSInstPrinter.h"
#include "llvm/Support/FormattedStream.h"

using namespace llvm;

// pin vtable to this file
MOSTargetStreamer::MOSTargetStreamer(MCStreamer &S) : MCTargetStreamer(S) {}

void MOSTargetStreamer::anchor() {}

MOSTargetAsmStreamer::MOSTargetAsmStreamer(MCStreamer &S,
                                               formatted_raw_ostream &OS)
    : MOSTargetStreamer(S), OS(OS) {}

void MOSTargetAsmStreamer::emitMOSRegisterIgnore(unsigned reg) {
  OS << "\t.register "
     << "%" << StringRef(MOSInstPrinter::getRegisterName(reg)).lower()
     << ", #ignore\n";
}

void MOSTargetAsmStreamer::emitMOSRegisterScratch(unsigned reg) {
  OS << "\t.register "
     << "%" << StringRef(MOSInstPrinter::getRegisterName(reg)).lower()
     << ", #scratch\n";
}

MOSTargetELFStreamer::MOSTargetELFStreamer(MCStreamer &S)
    : MOSTargetStreamer(S) {}

MCELFStreamer &MOSTargetELFStreamer::getStreamer() {
  return static_cast<MCELFStreamer &>(Streamer);
}
