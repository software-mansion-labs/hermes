/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-ir %s | %FileCheckOrRegen --match-full-lines %s

function dummy() {
  return;
}
function emptyString() {
  return dummy``;
}

function oneString() {
  return dummy`hello`;
}

function oneSub() {
  return dummy`${666}`;
}

function dup(x) {
  return dummy`hello world${1 + x}!!!`;
}

function notDup(x) {
  return dummy`hello\nworld${1 + x}!!!`;
}

function memberExpr() {
  var obj = {func: dummy};
  return obj.func`hello world!`;
}

function callExpr() {
  function func() {
    return function() {
      return;
    }
  }
  return func()`hello world!`;
}

/// Some more test cases to check template object id.

function dup2(x) {
  return dummy`hello world${1 + x}!!!`;
}

function dup3() {
  return dummy`hello world${7}!!!`;
}

function helloWorld() {
  return dummy`hello${0} world!!!`;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "dummy": string
// CHECK-NEXT:       DeclareGlobalVarInst "emptyString": string
// CHECK-NEXT:       DeclareGlobalVarInst "oneString": string
// CHECK-NEXT:       DeclareGlobalVarInst "oneSub": string
// CHECK-NEXT:       DeclareGlobalVarInst "dup": string
// CHECK-NEXT:       DeclareGlobalVarInst "notDup": string
// CHECK-NEXT:       DeclareGlobalVarInst "memberExpr": string
// CHECK-NEXT:       DeclareGlobalVarInst "callExpr": string
// CHECK-NEXT:       DeclareGlobalVarInst "dup2": string
// CHECK-NEXT:        DeclareGlobalVarInst "dup3": string
// CHECK-NEXT:        DeclareGlobalVarInst "helloWorld": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %0: environment, %VS0: any, %dummy(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "dummy": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %0: environment, %VS0: any, %emptyString(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "emptyString": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %0: environment, %VS0: any, %oneString(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %16: object, globalObject: object, "oneString": string
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %0: environment, %VS0: any, %oneSub(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %18: object, globalObject: object, "oneSub": string
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %0: environment, %VS0: any, %dup(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %20: object, globalObject: object, "dup": string
// CHECK-NEXT:  %22 = CreateFunctionInst (:object) %0: environment, %VS0: any, %notDup(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %22: object, globalObject: object, "notDup": string
// CHECK-NEXT:  %24 = CreateFunctionInst (:object) %0: environment, %VS0: any, %memberExpr(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %24: object, globalObject: object, "memberExpr": string
// CHECK-NEXT:  %26 = CreateFunctionInst (:object) %0: environment, %VS0: any, %callExpr(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %26: object, globalObject: object, "callExpr": string
// CHECK-NEXT:  %28 = CreateFunctionInst (:object) %0: environment, %VS0: any, %dup2(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %28: object, globalObject: object, "dup2": string
// CHECK-NEXT:  %30 = CreateFunctionInst (:object) %0: environment, %VS0: any, %dup3(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %30: object, globalObject: object, "dup3": string
// CHECK-NEXT:  %32 = CreateFunctionInst (:object) %0: environment, %VS0: any, %helloWorld(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %32: object, globalObject: object, "helloWorld": string
// CHECK-NEXT:  %34 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %34: any
// CHECK-NEXT:  %36 = LoadStackInst (:any) %34: any
// CHECK-NEXT:        ReturnInst %36: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 []

// CHECK:function dummy(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 []

// CHECK:function emptyString(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = GetTemplateObjectInst (:any) 0: number, true: boolean, "": string
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %2: any
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:scope %VS3 []

// CHECK:function oneString(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = GetTemplateObjectInst (:any) 1: number, true: boolean, "hello": string
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %2: any
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:scope %VS4 []

// CHECK:function oneSub(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:  %2 = GetTemplateObjectInst (:any) 2: number, true: boolean, "": string, "": string
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %2: any, 666: number
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:scope %VS5 [x: any]

// CHECK:function dup(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS5: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS5.x]: any
// CHECK-NEXT:  %4 = GetTemplateObjectInst (:any) 3: number, true: boolean, "hello world": string, "!!!": string
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %1: environment, [%VS5.x]: any
// CHECK-NEXT:  %6 = BinaryAddInst (:any) 1: number, %5: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %4: any, %6: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS6 [x: any]

// CHECK:function notDup(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS6: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS6.x]: any
// CHECK-NEXT:  %4 = GetTemplateObjectInst (:any) 4: number, false: boolean, "hello\\\\nworld": string, "!!!": string, "hello\\nworld": string, "!!!": string
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %1: environment, [%VS6.x]: any
// CHECK-NEXT:  %6 = BinaryAddInst (:any) 1: number, %5: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %4: any, %6: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS7 [obj: any]

// CHECK:function memberExpr(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS7: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS7.obj]: any
// CHECK-NEXT:  %3 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:  %4 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:       DefineNewOwnPropertyInst %4: any, %3: object, "func": string, true: boolean
// CHECK-NEXT:       StoreFrameInst %1: environment, %3: object, [%VS7.obj]: any
// CHECK-NEXT:  %7 = GetTemplateObjectInst (:any) 5: number, true: boolean, "hello world!": string
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS7.obj]: any
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "func": string
// CHECK-NEXT:  %10 = CallInst (:any) %9: any, empty: any, false: boolean, empty: any, undefined: undefined, %8: any, %7: any
// CHECK-NEXT:        ReturnInst %10: any
// CHECK-NEXT:function_end

// CHECK:scope %VS8 [func: any]

// CHECK:function callExpr(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS8: any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %VS8: any, %func(): functionCode
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: object, [%VS8.func]: any
// CHECK-NEXT:  %4 = GetTemplateObjectInst (:any) 5: number, true: boolean, "hello world!": string
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %1: environment, [%VS8.func]: any
// CHECK-NEXT:  %6 = CallInst (:any) %5: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %7 = CallInst (:any) %6: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %4: any
// CHECK-NEXT:       ReturnInst %7: any
// CHECK-NEXT:function_end

// CHECK:scope %VS9 [x: any]

// CHECK:function dup2(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS9: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS9.x]: any
// CHECK-NEXT:  %4 = GetTemplateObjectInst (:any) 3: number, true: boolean, "hello world": string, "!!!": string
// CHECK-NEXT:  %5 = LoadFrameInst (:any) %1: environment, [%VS9.x]: any
// CHECK-NEXT:  %6 = BinaryAddInst (:any) 1: number, %5: any
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %8 = CallInst (:any) %7: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %4: any, %6: any
// CHECK-NEXT:       ReturnInst %8: any
// CHECK-NEXT:function_end

// CHECK:scope %VS10 []

// CHECK:function dup3(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS10: any, %0: environment
// CHECK-NEXT:  %2 = GetTemplateObjectInst (:any) 3: number, true: boolean, "hello world": string, "!!!": string
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %2: any, 7: number
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:scope %VS11 []

// CHECK:function helloWorld(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS11: any, %0: environment
// CHECK-NEXT:  %2 = GetTemplateObjectInst (:any) 6: number, true: boolean, "hello": string, " world!!!": string
// CHECK-NEXT:  %3 = LoadPropertyInst (:any) globalObject: object, "dummy": string
// CHECK-NEXT:  %4 = CallInst (:any) %3: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %2: any, 0: number
// CHECK-NEXT:       ReturnInst %4: any
// CHECK-NEXT:function_end

// CHECK:scope %VS12 []

// CHECK:function func(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS8: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS12: any, %0: environment
// CHECK-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %VS12: any, %""(): functionCode
// CHECK-NEXT:       ReturnInst %2: object
// CHECK-NEXT:function_end

// CHECK:scope %VS13 []

// CHECK:function ""(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS12: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS13: any, %0: environment
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end
