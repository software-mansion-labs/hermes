/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -target=HBC -dump-lir %s | %FileCheckOrRegen --match-full-lines %s

// Test that DefineNewOwnPropertyInst is lowered to DefineOwnPropertyInst when
// the property name is a valid array index.
// We use a computed key to avoid emitting AllocObjectLiteral.

function foo() {
    return {a: 1, "10": 2, 11: 3, "999999999999999999999999": 4, ['42']: 5};
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %VS0: any, %foo(): functionCode
// CHECK-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:       StorePropertyLooseInst %2: object, %3: object, "foo": string
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  %6 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:       StoreStackInst %6: undefined, %5: any
// CHECK-NEXT:  %8 = LoadStackInst (:any) %5: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 []

// CHECK:function foo(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %3 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:       DefineNewOwnPropertyInst %3: number, %2: object, "a": string, true: boolean
// CHECK-NEXT:  %5 = HBCLoadConstInst (:number) 2: number
// CHECK-NEXT:       DefineNewOwnPropertyInst %5: number, %2: object, 10: number, true: boolean
// CHECK-NEXT:  %7 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:       DefineNewOwnPropertyInst %7: number, %2: object, 11: number, true: boolean
// CHECK-NEXT:  %9 = HBCLoadConstInst (:number) 4: number
// CHECK-NEXT:        DefineNewOwnPropertyInst %9: number, %2: object, "999999999999999999999999": string, true: boolean
// CHECK-NEXT:  %11 = HBCLoadConstInst (:number) 5: number
// CHECK-NEXT:        DefineOwnPropertyInst %11: number, %2: object, 42: number, true: boolean
// CHECK-NEXT:        ReturnInst %2: object
// CHECK-NEXT:function_end
