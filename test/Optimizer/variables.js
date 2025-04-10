/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s     -O  | %FileCheckOrRegen %s --match-full-lines --check-prefix=OPT-CHECK
// RUN: %hermesc -hermes-parser -dump-ir %s     -O0 | %FileCheckOrRegen %s --match-full-lines

function foo(p1, p2, p3) {
  var t = p1 + p2;
  var z = p2 + p3;
  var k = z + t;
  return ;
}

// Auto-generated content below. Please do not modify manually.

// OPT-CHECK:function global(): undefined
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// OPT-CHECK-NEXT:  %1 = CreateFunctionInst (:object) empty: any, empty: any, %foo(): functionCode
// OPT-CHECK-NEXT:       StorePropertyLooseInst %1: object, globalObject: object, "foo": string
// OPT-CHECK-NEXT:       ReturnInst undefined: undefined
// OPT-CHECK-NEXT:function_end

// OPT-CHECK:function foo(p1: any, p2: any, p3: any): undefined
// OPT-CHECK-NEXT:%BB0:
// OPT-CHECK-NEXT:  %0 = LoadParamInst (:any) %p1: any
// OPT-CHECK-NEXT:  %1 = LoadParamInst (:any) %p2: any
// OPT-CHECK-NEXT:  %2 = LoadParamInst (:any) %p3: any
// OPT-CHECK-NEXT:  %3 = BinaryAddInst (:string|number|bigint) %0: any, %1: any
// OPT-CHECK-NEXT:  %4 = BinaryAddInst (:string|number|bigint) %1: any, %2: any
// OPT-CHECK-NEXT:  %5 = BinaryAddInst (:string|number|bigint) %4: string|number|bigint, %3: string|number|bigint
// OPT-CHECK-NEXT:       ReturnInst undefined: undefined
// OPT-CHECK-NEXT:function_end

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHECK-NEXT:  %4 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %4: any
// CHECK-NEXT:  %6 = LoadStackInst (:any) %4: any
// CHECK-NEXT:       ReturnInst %6: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [p1: any, p2: any, p3: any, t: any, z: any, k: any]

// CHECK:function foo(p1: any, p2: any, p3: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %p1: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.p1]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %p2: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [%VS1.p2]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %p3: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [%VS1.p3]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.t]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.z]: any
// CHECK-NEXT:        StoreFrameInst %1: environment, undefined: undefined, [%VS1.k]: any
// CHECK-NEXT:  %11 = LoadFrameInst (:any) %1: environment, [%VS1.p1]: any
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS1.p2]: any
// CHECK-NEXT:  %13 = BinaryAddInst (:any) %11: any, %12: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %13: any, [%VS1.t]: any
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [%VS1.p2]: any
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %1: environment, [%VS1.p3]: any
// CHECK-NEXT:  %17 = BinaryAddInst (:any) %15: any, %16: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %17: any, [%VS1.z]: any
// CHECK-NEXT:  %19 = LoadFrameInst (:any) %1: environment, [%VS1.z]: any
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [%VS1.t]: any
// CHECK-NEXT:  %21 = BinaryAddInst (:any) %19: any, %20: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %21: any, [%VS1.k]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
