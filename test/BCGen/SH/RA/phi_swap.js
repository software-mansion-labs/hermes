/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O -dump-ra %s | %FileCheckOrRegen --match-full-lines %s
// Ensure that the register allocator correctly handles cycles between Phi-nodes.

function foo (a, b) {
    var t;
    for(;;) {
        t = a;
        a = b;
        b = t;
    }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:                 DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  {loc1}    %1 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  {loc0}    %2 = CreateFunctionInst (:object) empty: any, empty: any, %foo(): functionCode
// CHECK-NEXT:                 StorePropertyLooseInst {loc0} %2: object, {loc1} %1: object, "foo": string
// CHECK-NEXT:  {np0}     %4 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:                 ReturnInst {np0} %4: undefined
// CHECK-NEXT:function_end

// CHECK:function foo(a: any, b: any): any [noReturn]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  {loc1}    %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  {loc0}    %1 = LoadParamInst (:any) %b: any
// CHECK-NEXT:  {loc0}    %2 = MovInst (:any) {loc0} %1: any
// CHECK-NEXT:  {loc1}    %3 = MovInst (:any) {loc1} %0: any
// CHECK-NEXT:                 BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  {loc0}    %5 = PhiInst (:any) {loc0} %2: any, %BB0, {loc0} %9: any, %BB1
// CHECK-NEXT:  {loc1}    %6 = PhiInst (:any) {loc1} %3: any, %BB0, {loc1} %10: any, %BB1
// CHECK-NEXT:  {loc2}    %7 = MovInst (:any) {loc0} %5: any
// CHECK-NEXT:  {loc1}    %8 = MovInst (:any) {loc1} %6: any
// CHECK-NEXT:  {loc0}    %9 = MovInst (:any) {loc1} %8: any
// CHECK-NEXT:  {loc1}   %10 = MovInst (:any) {loc2} %7: any
// CHECK-NEXT:                 BranchInst %BB1
// CHECK-NEXT:function_end
