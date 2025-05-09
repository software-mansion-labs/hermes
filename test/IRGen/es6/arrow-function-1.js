/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

var func1 = () => 10;

var func2 = () => { return 11; }

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 [?anon_0_this: any, ?anon_1_new.target: undefined|object]

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:any) %<this>: any
// CHECK-NEXT:  %1 = CoerceThisNSInst (:object) %0: any
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       StoreFrameInst %2: environment, %1: object, [%VS0.?anon_0_this]: any
// CHECK-NEXT:  %4 = GetNewTargetInst (:undefined|object) %new.target: undefined|object
// CHECK-NEXT:       StoreFrameInst %2: environment, %4: undefined|object, [%VS0.?anon_1_new.target]: undefined|object
// CHECK-NEXT:       DeclareGlobalVarInst "func1": string
// CHECK-NEXT:       DeclareGlobalVarInst "func2": string
// CHECK-NEXT:  %8 = AllocStackInst (:any) $?anon_2_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %8: any
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %2: environment, %VS0: any, %func1(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "func1": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %2: environment, %VS0: any, %func2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "func2": string
// CHECK-NEXT:  %14 = LoadStackInst (:any) %8: any
// CHECK-NEXT:        ReturnInst %14: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 []

// CHECK:arrow func1(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       ReturnInst 10: number
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:arrow func2(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:       ReturnInst 11: number
// CHECK-NEXT:function_end
