/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -Werror -O0 -fno-inline -typed -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

'use strict';

class C {
  inherited(): number {
    return 1;
  }
}

class D extends C {
  constructor() {
    super();
  }
}

new D().inherited();

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

// CHECK:scope %VS1 [exports: any, C: any, D: any, ?C.prototype: object, ?D.prototype: object]

// CHECK:function ""(exports: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %exports: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.exports]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.C]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.D]: any
// CHECK-NEXT:  %6 = CreateFunctionInst (:object) %1: environment, %VS1: any, %C(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: object, [%VS1.C]: any
// CHECK-NEXT:  %8 = CreateFunctionInst (:object) %1: environment, %VS1: any, %inherited(): functionCode
// CHECK-NEXT:  %9 = AllocTypedObjectInst (:object) empty: any, "inherited": string, %8: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: object, [%VS1.?C.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %9: object, %6: object, "prototype": string
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS1.C]: any
// CHECK-NEXT:  %13 = CheckedTypeCastInst (:object) %12: any, type(object)
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %1: environment, %VS1: any, %D(): functionCode
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: object, [%VS1.D]: any
// CHECK-NEXT:  %16 = LoadFrameInst (:object) %1: environment, [%VS1.?C.prototype]: object
// CHECK-NEXT:  %17 = PrLoadInst (:object) %16: object, 0: number, "inherited": string
// CHECK-NEXT:  %18 = AllocTypedObjectInst (:object) %16: object, "inherited": string, %17: object
// CHECK-NEXT:        StoreFrameInst %1: environment, %18: object, [%VS1.?D.prototype]: object
// CHECK-NEXT:        StorePropertyStrictInst %18: object, %14: object, "prototype": string
// CHECK-NEXT:  %21 = LoadFrameInst (:any) %1: environment, [%VS1.D]: any
// CHECK-NEXT:  %22 = CheckedTypeCastInst (:object) %21: any, type(object)
// CHECK-NEXT:  %23 = LoadFrameInst (:object) %1: environment, [%VS1.?D.prototype]: object
// CHECK-NEXT:  %24 = UnionNarrowTrustedInst (:object) %23: object
// CHECK-NEXT:  %25 = AllocTypedObjectInst (:object) %24: object
// CHECK-NEXT:  %26 = CallInst (:any) %22: object, %D(): functionCode, true: boolean, empty: any, %22: object, %25: object
// CHECK-NEXT:  %27 = TypedLoadParentInst (:object) %25: object
// CHECK-NEXT:  %28 = PrLoadInst (:object) %27: object, 0: number, "inherited": string
// CHECK-NEXT:  %29 = CallInst [njsf] (:any) %28: object, %inherited(): functionCode, true: boolean, empty: any, undefined: undefined, %25: object
// CHECK-NEXT:  %30 = CheckedTypeCastInst (:number) %29: any, type(number)
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:function C(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:function inherited(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:       ReturnInst 1: number
// CHECK-NEXT:function_end

// CHECK:scope %VS4 []

// CHECK:derived constructor D(): any [typed]
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = LoadParamInst (:object) %<this>: object
// CHECK-NEXT:  %1 = GetParentScopeInst (:environment) %VS1: any, %parentScope: environment
// CHECK-NEXT:  %2 = CreateScopeInst (:environment) %VS4: any, %1: environment
// CHECK-NEXT:  %3 = LoadFrameInst (:any) %1: environment, [%VS1.C]: any
// CHECK-NEXT:  %4 = CheckedTypeCastInst (:object) %3: any, type(object)
// CHECK-NEXT:  %5 = GetNewTargetInst (:object) %new.target: object
// CHECK-NEXT:  %6 = CallInst (:any) %4: object, empty: any, false: boolean, empty: any, %5: object, %0: object
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
