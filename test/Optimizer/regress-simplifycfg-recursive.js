/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-ir %s -fno-inline | %FileCheckOrRegen %s --match-full-lines

// Test that we can delete recursive functions after they're unused.
// Their only users are themselves.

(function main() {
  'use strict';
  var b = true;
  if (!b) {
    function foo() {
      foo();
    }
  }
});

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): object
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateFunctionInst (:object) empty: any, empty: any, %main(): functionCode
// CHECK-NEXT:       ReturnInst %0: object
// CHECK-NEXT:function_end

// CHECK:function main(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
