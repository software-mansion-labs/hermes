/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -dump-ir %s -O | %FileCheckOrRegen %s --match-full-lines

'use strict'
(function main() {
  // FunctionAnalysis was crashing because the users list was changing
  // as we iterated the users of a function to analyze callsites.
  function f1() {
    return f2(f1);
  }
  function f2(cb) {
    if (a) {
      return b ? 0 : cb();
    }
    return cb();
  }
  function f4() {
    f1();
  }
  function f5() {
    f1();
  }
  function f6() {
    f4();
  }
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 [f1: object]

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) %0: environment, %VS0: any, %f1(): functionCode
// CHECK-NEXT:       StoreFrameInst %0: environment, %1: object, [%VS0.f1]: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function f1(): number [allCallsitesKnownInStrictMode]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = LoadFrameInst (:object) %0: environment, [%VS0.f1]: object
// CHECK-NEXT:  %2 = TryLoadGlobalPropertyInst (:any) globalObject: object, "a": string
// CHECK-NEXT:       CondBranchInst %2: any, %BB3, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %4 = PhiInst (:number) %6: number, %BB2, %12: number, %BB5
// CHECK-NEXT:       ReturnInst %4: number
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  %6 = CallInst (:number) %1: object, %f1(): functionCode, true: boolean, %0: environment, undefined: undefined, 0: number
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %8 = TryLoadGlobalPropertyInst (:any) globalObject: object, "b": string
// CHECK-NEXT:       CondBranchInst %8: any, %BB5, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = CallInst (:number) %1: object, %f1(): functionCode, true: boolean, %0: environment, undefined: undefined, 0: number
// CHECK-NEXT:        BranchInst %BB5
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %12 = PhiInst (:number) %10: number, %BB4, 0: number, %BB3
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end
