/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function simple_test0() {
  var re = /(\w+)\s(\w+)/;
  var str = 'John Smith';
  var newstr = str.replace(re, '$2, $1');
}

function simple_test1() {
  var re0 = /\w+\s/g;
  var re1 = /\w+\s/g;
  var re2 = /\w+/;
  var re3 = /\w+/g;
  var re4 = / /;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "simple_test0": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_test1": string
// CHECK-NEXT:  %3 = CreateFunctionInst (:object) %0: environment, %VS0: any, %simple_test0(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %3: object, globalObject: object, "simple_test0": string
// CHECK-NEXT:  %5 = CreateFunctionInst (:object) %0: environment, %VS0: any, %simple_test1(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %5: object, globalObject: object, "simple_test1": string
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:       StoreStackInst undefined: undefined, %7: any
// CHECK-NEXT:  %9 = LoadStackInst (:any) %7: any
// CHECK-NEXT:        ReturnInst %9: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [re: any, str: any, newstr: any]

// CHECK:function simple_test0(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.re]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.str]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS1.newstr]: any
// CHECK-NEXT:  %5 = CreateRegExpInst (:object) "(\\\\w+)\\\\s(\\\\w+)": string, "": string
// CHECK-NEXT:       StoreFrameInst %1: environment, %5: object, [%VS1.re]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, "John Smith": string, [%VS1.str]: any
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS1.str]: any
// CHECK-NEXT:  %9 = LoadPropertyInst (:any) %8: any, "replace": string
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS1.re]: any
// CHECK-NEXT:  %11 = CallInst (:any) %9: any, empty: any, false: boolean, empty: any, undefined: undefined, %8: any, %10: any, "$2, $1": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: any, [%VS1.newstr]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [re0: any, re1: any, re2: any, re3: any, re4: any]

// CHECK:function simple_test1(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.re0]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.re1]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.re2]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.re3]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.re4]: any
// CHECK-NEXT:  %7 = CreateRegExpInst (:object) "\\\\w+\\\\s": string, "g": string
// CHECK-NEXT:       StoreFrameInst %1: environment, %7: object, [%VS2.re0]: any
// CHECK-NEXT:  %9 = CreateRegExpInst (:object) "\\\\w+\\\\s": string, "g": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %9: object, [%VS2.re1]: any
// CHECK-NEXT:  %11 = CreateRegExpInst (:object) "\\\\w+": string, "": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %11: object, [%VS2.re2]: any
// CHECK-NEXT:  %13 = CreateRegExpInst (:object) "\\\\w+": string, "g": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %13: object, [%VS2.re3]: any
// CHECK-NEXT:  %15 = CreateRegExpInst (:object) " ": string, "": string
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: object, [%VS2.re4]: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end
