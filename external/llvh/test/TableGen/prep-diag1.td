// RUN: (! %llvm-tblgen -DDIAG1 -I %p %s 2>&1) | %FileCheck --check-prefixes=DIAG1 %s
// RUN: (! %llvm-tblgen -DDIAG4 -I %p %s 2>&1) | %FileCheck --check-prefixes=DIAG4 %s
// RUN: (! %llvm-tblgen -DDIAG2 -I %p %s 2>&1) | %FileCheck --check-prefixes=DIAG2 %s
// RUN: (! %llvm-tblgen -I %p %s 2>&1) | %FileCheck --check-prefixes=DIAG3 %s

#ifdef DIAG1
// DIAG1: error: Only comments are supported after #define NAME
#define ENABLED1/*
*/class C;
#endif // DIAG1

#ifdef DIAG4
// DIAG4: warning: Duplicate definition of macro: ENABLED1
#define ENABLED1
#define ENABLED1
#endif // DIAG4

#ifdef DIAG2
// DIAG2: error: Only comments are supported after #ifdef NAME

// Invalid #ifdef below should be detected even if DIAG2 is not defined.
// DIAG3: error: Only comments are supported after #ifdef NAME
#ifdef DIAG2/*
*/class C;
#endif
#endif // DIAG2
