// RUN: (! %llvm-tblgen -DDIAG1 -I %p %s 2>&1) | %FileCheck --check-prefixes=DIAG1 %s
// RUN: (! %llvm-tblgen -I %p %s 2>&1) | %FileCheck --check-prefixes=DIAG2 %s

#ifdef DIAG1
// DIAG1: error: Only comments are supported after #endif

// Invalid #else below should be detected even if DIAG1 is not defined.
// DIAG2: error: Only comments are supported after #endif
#ifdef DIAG2//DIAG2
#else/*!DIAG2*/
#endif/* !DIAG2
*/class C;
#endif // DIAG1

