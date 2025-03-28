//===- llvh/TableGen/Error.h - tblgen error handling helpers ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains error handling helper routines to pretty-print diagnostic
// messages from tblgen.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TABLEGEN_ERROR_H
#define LLVM_TABLEGEN_ERROR_H

#include "llvh/Support/SourceMgr.h"

namespace llvh {

void PrintNote(ArrayRef<SMLoc> NoteLoc, const Twine &Msg);

void PrintWarning(ArrayRef<SMLoc> WarningLoc, const Twine &Msg);
void PrintWarning(const char *Loc, const Twine &Msg);
void PrintWarning(const Twine &Msg);

void PrintError(ArrayRef<SMLoc> ErrorLoc, const Twine &Msg);
void PrintError(const char *Loc, const Twine &Msg);
void PrintError(const Twine &Msg);

LLVM_ATTRIBUTE_NORETURN void PrintFatalError(const Twine &Msg);
LLVM_ATTRIBUTE_NORETURN void PrintFatalError(ArrayRef<SMLoc> ErrorLoc,
                                             const Twine &Msg);

extern SourceMgr SrcMgr;
extern unsigned ErrorsPrinted;

} // end namespace "llvh"

#endif
