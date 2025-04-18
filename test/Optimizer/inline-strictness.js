/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function outer(a, b) {
    function f1() {
        'use strict';
        return f2() + 1;
    }
    function f2() {
        return a;
    }
    return f1();
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "outer": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %outer(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "outer": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function outer(a: any, b: any): string|number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %a: any
// CHECK-NEXT:  %1 = BinaryAddInst (:string|number) %0: any, 1: number
// CHECK-NEXT:       ReturnInst %1: string|number
// CHECK-NEXT:function_end
