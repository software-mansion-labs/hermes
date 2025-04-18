/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O -dump-ir -Xno-dump-functions=global %s | %FileCheck --match-full-lines %s
// RUN: %shermes -O -dump-ir -Xno-dump-functions=global -fno-inline %s | %FileCheck --match-full-lines --check-prefix=CHK2 %s
// XFAIL: true
// TODO: This test fails because we disable try/catch inlining.

// Ensure that ThrowInst targets are correctly set when inlined into a try/catch.

function outer(a) {
    'use strict'

    function thrower() {
        throw "Hello!";
    }

    try {
        thrower();
    } catch(e) {
        print(e);
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function outer(a: any): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       TryStartInst %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = CatchInst (:any)
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHECK-NEXT:  %3 = CallInst (:any) %2: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %1: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ThrowInst "Hello!": string, %BB1
// CHECK-NEXT:function_end

// CHK2:function outer(a: any): undefined
// CHK2-NEXT:%BB0:
// CHK2-NEXT:  %0 = CreateFunctionInst (:object) empty: any, %thrower(): functionCode
// CHK2-NEXT:       TryStartInst %BB1, %BB2
// CHK2-NEXT:%BB1:
// CHK2-NEXT:  %2 = CatchInst (:any)
// CHK2-NEXT:  %3 = TryLoadGlobalPropertyInst (:any) globalObject: object, "print": string
// CHK2-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %2: any
// CHK2-NEXT:       ReturnInst undefined: undefined
// CHK2-NEXT:%BB2:
// CHK2-NEXT:  %6 = CallInst (:any) %0: object, %thrower(): functionCode, true: boolean, empty: any, undefined: undefined, 0: number
// CHK2-NEXT:       UnreachableInst
// CHK2-NEXT:function_end

// CHK2:function thrower(): any [allCallsitesKnownInStrictMode,noReturn]
// CHK2-NEXT:%BB0:
// CHK2-NEXT:       ThrowInst "Hello!": string
// CHK2-NEXT:function_end
