/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

(function main() {
  for (var v1 = 0;; v1++) {
    switch (v1 === "") {
      case 0:
        for (var v10 = 0; v10 < 1; v10++) {
          switch (v1) {
            case 0:
              var v12 = 0;
              var v13 = 10;
              while (v12 < v13) {}
          }
        }
        break;
      default:
        break;
    }
  }
})();

// Auto-generated content below. Please do not modify manually.

// CHECK:function global(): any [noReturn]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %1 = PhiInst (:number) 0: number, %BB0, %2: number, %BB1
// CHECK-NEXT:  %2 = FAddInst (:number) %1: number, 1: number
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:function_end
