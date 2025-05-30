/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir -strict %s 2>&1 | %FileCheckOrRegen %s --match-full-lines

var x = y;

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}undeclared_strict.js:10:9: warning: the variable "y" was not declared in function "global"
// CHECK-NEXT:var x = y;
// CHECK-NEXT:        ^
// CHECK-NEXT:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "x": string
// CHECK-NEXT:  %2 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %2: any
// CHECK-NEXT:  %4 = TryLoadGlobalPropertyInst (:any) globalObject: object, "y": string
// CHECK-NEXT:       StorePropertyStrictInst %4: any, globalObject: object, "x": string
// CHECK-NEXT:  %6 = LoadStackInst (:any) %2: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end
