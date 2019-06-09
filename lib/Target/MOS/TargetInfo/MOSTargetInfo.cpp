//===-- MOSTargetInfo.cpp - MOS Target Implementation -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/MOSTargetInfo.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheMOSTarget() {
  static Target TheMOSTarget;
  return TheMOSTarget;
}
Target &llvm::getTheMOSV9Target() {
  static Target TheMOSV9Target;
  return TheMOSV9Target;
}
Target &llvm::getTheMOSelTarget() {
  static Target TheMOSelTarget;
  return TheMOSelTarget;
}

extern "C" void LLVMInitializeMOSTargetInfo() {
  RegisterTarget<Triple::mos, /*HasJIT=*/true> X(getTheMOSTarget(), "mos",
                                                   "MOS", "MOS");
  RegisterTarget<Triple::mosv9, /*HasJIT=*/true> Y(
      getTheMOSV9Target(), "mosv9", "MOS V9", "MOS");
  RegisterTarget<Triple::mosel, /*HasJIT=*/true> Z(
      getTheMOSelTarget(), "mosel", "MOS LE", "MOS");
}
