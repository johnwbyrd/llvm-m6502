//===-- MOSTargetInfo.h - MOS Target Implementation ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_MOS_TARGETINFO_MOSTARGETINFO_H
#define LLVM_LIB_TARGET_MOS_TARGETINFO_MOSTARGETINFO_H

namespace llvm {

class Target;

Target &getTheMOSTarget();
Target &getTheMOSV9Target();
Target &getTheMOSelTarget();

} // namespace llvm

#endif // LLVM_LIB_TARGET_MOS_TARGETINFO_MOSTARGETINFO_H
