/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s     -O | %FileCheckOrRegen %s --match-full-lines

// Make sure we are not removing the binary operator. "In" may throw.
function test0() {
  "foo" in true;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       DeclareGlobalVarInst "test0": string
// CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %test0(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "test0": string
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:function test0(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = BinaryInInst (:boolean) "foo": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
