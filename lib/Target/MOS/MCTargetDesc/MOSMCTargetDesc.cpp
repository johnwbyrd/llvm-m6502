//===-- MOSMCTargetDesc.cpp - MOS Target Descriptions -----------------===//
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

#include "MOSMCTargetDesc.h"
#include "MOSInstPrinter.h"
#include "MOSMCAsmInfo.h"
#include "MOSTargetStreamer.h"
#include "TargetInfo/MOSTargetInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define GET_INSTRINFO_MC_DESC
#include "MOSGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "MOSGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "MOSGenRegisterInfo.inc"

static MCAsmInfo *createMOSMCAsmInfo(const MCRegisterInfo &MRI,
                                       const Triple &TT) {
  MCAsmInfo *MAI = new MOSELFMCAsmInfo(TT);
  unsigned Reg = MRI.getDwarfRegNum(SP::O6, true);
  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(nullptr, Reg, 0);
  MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCAsmInfo *createMOSV9MCAsmInfo(const MCRegisterInfo &MRI,
                                         const Triple &TT) {
  MCAsmInfo *MAI = new MOSELFMCAsmInfo(TT);
  unsigned Reg = MRI.getDwarfRegNum(SP::O6, true);
  MCCFIInstruction Inst = MCCFIInstruction::createDefCfa(nullptr, Reg, 2047);
  MAI->addInitialFrameState(Inst);
  return MAI;
}

static MCInstrInfo *createMOSMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitMOSMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createMOSMCRegisterInfo(const Triple &TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitMOSMCRegisterInfo(X, SP::O7);
  return X;
}

static MCSubtargetInfo *
createMOSMCSubtargetInfo(const Triple &TT, StringRef CPU, StringRef FS) {
  if (CPU.empty())
    CPU = (TT.getArch() == Triple::mosv9) ? "v9" : "v8";
  return createMOSMCSubtargetInfoImpl(TT, CPU, FS);
}

static MCTargetStreamer *
createObjectTargetStreamer(MCStreamer &S, const MCSubtargetInfo &STI) {
  return new MOSTargetELFStreamer(S);
}

static MCTargetStreamer *createTargetAsmStreamer(MCStreamer &S,
                                                 formatted_raw_ostream &OS,
                                                 MCInstPrinter *InstPrint,
                                                 bool isVerboseAsm) {
  return new MOSTargetAsmStreamer(S, OS);
}

static MCInstPrinter *createMOSMCInstPrinter(const Triple &T,
                                               unsigned SyntaxVariant,
                                               const MCAsmInfo &MAI,
                                               const MCInstrInfo &MII,
                                               const MCRegisterInfo &MRI) {
  return new MOSInstPrinter(MAI, MII, MRI);
}

extern "C" void LLVMInitializeMOSTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfoFn X(getTheMOSTarget(), createMOSMCAsmInfo);
  RegisterMCAsmInfoFn Y(getTheMOSV9Target(), createMOSV9MCAsmInfo);
  RegisterMCAsmInfoFn Z(getTheMOSelTarget(), createMOSMCAsmInfo);

  for (Target *T :
       {&getTheMOSTarget(), &getTheMOSV9Target(), &getTheMOSelTarget()}) {
    // Register the MC instruction info.
    TargetRegistry::RegisterMCInstrInfo(*T, createMOSMCInstrInfo);

    // Register the MC register info.
    TargetRegistry::RegisterMCRegInfo(*T, createMOSMCRegisterInfo);

    // Register the MC subtarget info.
    TargetRegistry::RegisterMCSubtargetInfo(*T, createMOSMCSubtargetInfo);

    // Register the MC Code Emitter.
    TargetRegistry::RegisterMCCodeEmitter(*T, createMOSMCCodeEmitter);

    // Register the asm backend.
    TargetRegistry::RegisterMCAsmBackend(*T, createMOSAsmBackend);

    // Register the object target streamer.
    TargetRegistry::RegisterObjectTargetStreamer(*T,
                                                 createObjectTargetStreamer);

    // Register the asm streamer.
    TargetRegistry::RegisterAsmTargetStreamer(*T, createTargetAsmStreamer);

    // Register the MCInstPrinter
    TargetRegistry::RegisterMCInstPrinter(*T, createMOSMCInstPrinter);
  }
}
