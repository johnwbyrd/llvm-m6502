//===-- MOSMCTargetDesc.h - MOS Target Descriptions ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides MOS specific target descriptions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MOS_MCTARGETDESC_MOSMCTARGETDESC_H
#define LLVM_LIB_TARGET_MOS_MCTARGETDESC_MOSMCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

#include <memory>

namespace llvm {
class MCAsmBackend;
class MCCodeEmitter;
class MCContext;
class MCInstrInfo;
class MCObjectTargetWriter;
class MCRegisterInfo;
class MCSubtargetInfo;
class MCTargetOptions;
class Target;
class Triple;
class StringRef;
class raw_pwrite_stream;
class raw_ostream;

MCCodeEmitter *createMOSMCCodeEmitter(const MCInstrInfo &MCII,
                                        const MCRegisterInfo &MRI,
                                        MCContext &Ctx);
MCAsmBackend *createMOSAsmBackend(const Target &T, const MCSubtargetInfo &STI,
                                    const MCRegisterInfo &MRI,
                                    const MCTargetOptions &Options);
std::unique_ptr<MCObjectTargetWriter> createMOSELFObjectWriter(bool Is64Bit,
                                                                 uint8_t OSABI);
} // End llvm namespace

// Defines symbolic names for MOS registers.  This defines a mapping from
// register name to register number.
//
#define GET_REGINFO_ENUM
#include "MOSGenRegisterInfo.inc"

// Defines symbolic names for the MOS instructions.
//
#define GET_INSTRINFO_ENUM
#include "MOSGenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "MOSGenSubtargetInfo.inc"

#endif
