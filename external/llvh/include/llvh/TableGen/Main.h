//===- llvh/TableGen/Main.h - tblgen entry point ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the common entry point for tblgen tools.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TABLEGEN_MAIN_H
#define LLVM_TABLEGEN_MAIN_H

namespace llvh {

class raw_ostream;
class RecordKeeper;

/// Perform the action using Records, and write output to OS.
/// Returns true on error, false otherwise.
using TableGenMainFn = bool (raw_ostream &OS, RecordKeeper &Records);

int TableGenMain(char *argv0, TableGenMainFn *MainFn);

} // end namespace llvh

#endif // LLVM_TABLEGEN_MAIN_H
