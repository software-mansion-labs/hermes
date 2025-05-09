/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function sink(a) { }

function simple_for_loop() {
  for (var i = 0; i < 10; i = i + 1) { sink(i) }
}

function simple_for_loop_break() {
  for (var i = 0; i < 10; i = i + 1) { break; }
}

function simple_for_loop_break_label() {
  fail:
  for (var i = 0; i < 10; i = i + 1) { break fail; }
}

function simple_for_loop_continue() {
  for (var i = 0; i < 10; i = i + 1) { continue; }
}

function simple_for_loop_continue_label() {
  fail:
  for (var i = 0; i < 10; i = i + 1) { continue fail; }
}

function for_loop_match(a,b,c,d,e,f) {
  for (a(); b(); c()) { d(); break; e(); }
}

function naked_for_loop() {
  for (;;) { }
}

// Make sure we are not crashing on expressions in the update and init field.
function test_init_update_exprs(param1) {
  for (var i = 0; false ; i++) { }
  for (4        ; false ; --i) { }
  for (param1   ; false ; 2)   { }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "sink": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_for_loop": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_for_loop_break": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_for_loop_break_label": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_for_loop_continue": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_for_loop_continue_label": string
// CHECK-NEXT:       DeclareGlobalVarInst "for_loop_match": string
// CHECK-NEXT:       DeclareGlobalVarInst "naked_for_loop": string
// CHECK-NEXT:       DeclareGlobalVarInst "test_init_update_exprs": string
// CHECK-NEXT:  %10 = CreateFunctionInst (:object) %0: environment, %VS0: any, %sink(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %10: object, globalObject: object, "sink": string
// CHECK-NEXT:  %12 = CreateFunctionInst (:object) %0: environment, %VS0: any, %simple_for_loop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %12: object, globalObject: object, "simple_for_loop": string
// CHECK-NEXT:  %14 = CreateFunctionInst (:object) %0: environment, %VS0: any, %simple_for_loop_break(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %14: object, globalObject: object, "simple_for_loop_break": string
// CHECK-NEXT:  %16 = CreateFunctionInst (:object) %0: environment, %VS0: any, %simple_for_loop_break_label(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %16: object, globalObject: object, "simple_for_loop_break_label": string
// CHECK-NEXT:  %18 = CreateFunctionInst (:object) %0: environment, %VS0: any, %simple_for_loop_continue(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %18: object, globalObject: object, "simple_for_loop_continue": string
// CHECK-NEXT:  %20 = CreateFunctionInst (:object) %0: environment, %VS0: any, %simple_for_loop_continue_label(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %20: object, globalObject: object, "simple_for_loop_continue_label": string
// CHECK-NEXT:  %22 = CreateFunctionInst (:object) %0: environment, %VS0: any, %for_loop_match(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %22: object, globalObject: object, "for_loop_match": string
// CHECK-NEXT:  %24 = CreateFunctionInst (:object) %0: environment, %VS0: any, %naked_for_loop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %24: object, globalObject: object, "naked_for_loop": string
// CHECK-NEXT:  %26 = CreateFunctionInst (:object) %0: environment, %VS0: any, %test_init_update_exprs(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %26: object, globalObject: object, "test_init_update_exprs": string
// CHECK-NEXT:  %28 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %28: any
// CHECK-NEXT:  %30 = LoadStackInst (:any) %28: any
// CHECK-NEXT:        ReturnInst %30: any
// CHECK-NEXT:function_end

// CHECK:scope %VS1 [a: any]

// CHECK:function sink(a: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS1: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS1.a]: any
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS2 [i: any]

// CHECK:function simple_for_loop(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS2.i]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS2.i]: any
// CHECK-NEXT:  %5 = BinaryLessThanInst (:boolean) %4: any, 10: number
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %7 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %8 = LoadFrameInst (:any) %1: environment, [%VS2.i]: any
// CHECK-NEXT:  %9 = CallInst (:any) %7: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %8: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS2.i]: any
// CHECK-NEXT:  %13 = BinaryLessThanInst (:boolean) %12: any, 10: number
// CHECK-NEXT:        CondBranchInst %13: boolean, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %15 = LoadFrameInst (:any) %1: environment, [%VS2.i]: any
// CHECK-NEXT:  %16 = BinaryAddInst (:any) %15: any, 1: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %16: any, [%VS2.i]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [i: any]

// CHECK:function simple_for_loop_break(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS3.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS3.i]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS3.i]: any
// CHECK-NEXT:  %5 = BinaryLessThanInst (:boolean) %4: any, 10: number
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [i: any]

// CHECK:function simple_for_loop_break_label(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS4.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS4.i]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS4.i]: any
// CHECK-NEXT:  %5 = BinaryLessThanInst (:boolean) %4: any, 10: number
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:scope %VS5 [i: any]

// CHECK:function simple_for_loop_continue(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS5: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS5.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS5.i]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %5 = BinaryLessThanInst (:boolean) %4: any, 10: number
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %10 = BinaryLessThanInst (:boolean) %9: any, 10: number
// CHECK-NEXT:        CondBranchInst %10: boolean, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %12 = LoadFrameInst (:any) %1: environment, [%VS5.i]: any
// CHECK-NEXT:  %13 = BinaryAddInst (:any) %12: any, 1: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %13: any, [%VS5.i]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:function_end

// CHECK:scope %VS6 [i: any]

// CHECK:function simple_for_loop_continue_label(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS6: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS6.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS6.i]: any
// CHECK-NEXT:  %4 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %5 = BinaryLessThanInst (:boolean) %4: any, 10: number
// CHECK-NEXT:       CondBranchInst %5: boolean, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB5
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %11 = BinaryLessThanInst (:boolean) %10: any, 10: number
// CHECK-NEXT:        CondBranchInst %11: boolean, %BB2, %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:  %13 = LoadFrameInst (:any) %1: environment, [%VS6.i]: any
// CHECK-NEXT:  %14 = BinaryAddInst (:any) %13: any, 1: number
// CHECK-NEXT:        StoreFrameInst %1: environment, %14: any, [%VS6.i]: any
// CHECK-NEXT:        BranchInst %BB4
// CHECK-NEXT:function_end

// CHECK:scope %VS7 [a: any, b: any, c: any, d: any, e: any, f: any]

// CHECK:function for_loop_match(a: any, b: any, c: any, d: any, e: any, f: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS7: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %a: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS7.a]: any
// CHECK-NEXT:  %4 = LoadParamInst (:any) %b: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %4: any, [%VS7.b]: any
// CHECK-NEXT:  %6 = LoadParamInst (:any) %c: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %6: any, [%VS7.c]: any
// CHECK-NEXT:  %8 = LoadParamInst (:any) %d: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %8: any, [%VS7.d]: any
// CHECK-NEXT:  %10 = LoadParamInst (:any) %e: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %10: any, [%VS7.e]: any
// CHECK-NEXT:  %12 = LoadParamInst (:any) %f: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %12: any, [%VS7.f]: any
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [%VS7.a]: any
// CHECK-NEXT:  %15 = CallInst (:any) %14: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %1: environment, [%VS7.b]: any
// CHECK-NEXT:  %17 = CallInst (:any) %16: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        CondBranchInst %17: any, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %19 = LoadFrameInst (:any) %1: environment, [%VS7.d]: any
// CHECK-NEXT:  %20 = CallInst (:any) %19: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:function_end

// CHECK:scope %VS8 []

// CHECK:function naked_for_loop(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS8: any, %0: environment
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:scope %VS9 [param1: any, i: any]

// CHECK:function test_init_update_exprs(param1: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS9: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %param1: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS9.param1]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS9.i]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS9.i]: any
// CHECK-NEXT:       CondBranchInst false: boolean, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:       BranchInst %BB4
// CHECK-NEXT:%BB2:
// CHECK-NEXT:       CondBranchInst false: boolean, %BB5, %BB6
// CHECK-NEXT:%BB3:
// CHECK-NEXT:       CondBranchInst false: boolean, %BB1, %BB2
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS9.i]: any
// CHECK-NEXT:  %11 = AsNumericInst (:number|bigint) %10: any
// CHECK-NEXT:  %12 = UnaryIncInst (:number|bigint) %11: number|bigint
// CHECK-NEXT:        StoreFrameInst %1: environment, %12: number|bigint, [%VS9.i]: any
// CHECK-NEXT:        BranchInst %BB3
// CHECK-NEXT:%BB5:
// CHECK-NEXT:        BranchInst %BB8
// CHECK-NEXT:%BB6:
// CHECK-NEXT:  %16 = LoadFrameInst (:any) %1: environment, [%VS9.param1]: any
// CHECK-NEXT:        CondBranchInst false: boolean, %BB9, %BB10
// CHECK-NEXT:%BB7:
// CHECK-NEXT:        CondBranchInst false: boolean, %BB5, %BB6
// CHECK-NEXT:%BB8:
// CHECK-NEXT:  %19 = LoadFrameInst (:any) %1: environment, [%VS9.i]: any
// CHECK-NEXT:  %20 = UnaryDecInst (:number|bigint) %19: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %20: number|bigint, [%VS9.i]: any
// CHECK-NEXT:        BranchInst %BB7
// CHECK-NEXT:%BB9:
// CHECK-NEXT:        BranchInst %BB12
// CHECK-NEXT:%BB10:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB11:
// CHECK-NEXT:        CondBranchInst false: boolean, %BB9, %BB10
// CHECK-NEXT:%BB12:
// CHECK-NEXT:        BranchInst %BB11
// CHECK-NEXT:function_end
