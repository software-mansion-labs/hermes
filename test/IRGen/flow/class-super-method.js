/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -typed -dump-ir -O0 %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

class A {
  x: number;

  constructor(x: number) {
    this.x = x;
  }

  f(): number {
    return this.x * 10;
  }
}

class B extends A {
  constructor(x: number) {
    super(x);
  }

  f(): number {
    return super.f() + 23;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  %1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %1: any
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %""(): functionCode
// CHECK-NEXT:  %4 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %3: object, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %4: object
// CHECK-NEXT:       StoreStackInst %5: any, %1: any
// CHECK-NEXT:  %7 = LoadStackInst (:any) %1: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [exports: any, A: any, B: any, ?A.prototype: object, ?B.prototype: object]

// CHECK:function ""(exports: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.A]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.B]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %VS1: any, %A(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [%VS1.A]: any
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %VS1: any, %f(): functionCode
// CHECK-NEXT:  %9 = AllocTypedObjectInst (:object) empty: any, "f": string, %8: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: object, [%VS1.?A.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %9: object, %6: object, "prototype": string
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS1.A]: any
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:object) %12: any, type(object)
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %1: environment, %VS1: any, %B(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: object, [%VS1.B]: any
// CHECK-NEXT:  %16 = LoadFrameInst (:object) %1: environment, [%VS1.?A.prototype]: object
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %1: environment, %VS1: any, %"f 1#"(): functionCode
// CHECK-NEXT:  %18 = AllocTypedObjectInst (:object) %16: object, "f": string, %17: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: object, [%VS1.?B.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %18: object, %14: object, "prototype": string
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [x: any]

// CHECK:base constructor A(x: number): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS2: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: number, [%VS2.x]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %2: environment, [%VS2.x]: any
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:number) %5: any, type(number)
// CHECK-NEXT:       PrStoreInst %6: number, %0: object, 0: number, "x": string, true: boolean
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:function f(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS3: any, %1: environment
// CHECK-NEXT:  %3 = PrLoadInst (:number) %0: object, 0: number, "x": string
// CHECK-NEXT:  %4 = BinaryMultiplyInst (:any) %3: number, 10: number
// CHECK-NEXT:  %5 = CheckedTypeCastInst (:number) %4: any, type(number)
// CHECK-NEXT:       ReturnInst %5: number
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [x: any]

// CHECK:derived constructor B(x: number): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS4: any, %1: environment
// CHECK-NEXT:  %3 = LoadParamInst (:number) %x: number
// CHECK-NEXT:       StoreFrameInst %2: environment, %3: number, [%VS4.x]: any
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %1: environment, [%VS1.A]: any
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:object) %5: any, type(object)
// CHECK-NEXT:  %7 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %2: environment, [%VS4.x]: any
// CHECK-NEXT:  %9 = CheckedTypeCastInst (:number) %8: any, type(number)
// CHECK-NEXT:  %10 = CallInst [njsf] (:any) %6: object, empty: any, false: boolean, empty: any, %7: object, %0: object, %9: number
// CHECK-NEXT:  %11 = CheckedTypeCastInst (:undefined) %10: any, type(undefined)
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS5 []

// CHECK:function "f 1#"(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS5: any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:object) %1: environment, [%VS1.?A.prototype]: object
// CHECK-NEXT:  %4 = PrLoadInst (:object) %3: object, 0: number, "f": string
// CHECK-NEXT:  %5 = CallInst [njsf] (:any) %4: object, empty: any, false: boolean, empty: any, undefined: undefined, %0: object
// CHECK-NEXT:  %6 = CheckedTypeCastInst (:number) %5: any, type(number)
// CHECK-NEXT:  %7 = BinaryAddInst (:any) %6: number, 23: number
// CHECK-NEXT:  %8 = CheckedTypeCastInst (:number) %7: any, type(number)
// CHECK-NEXT:       ReturnInst %8: number
// CHECK-NEXT:function_end
