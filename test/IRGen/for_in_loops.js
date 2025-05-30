/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -hermes-parser -dump-ir %s -O0 | %FileCheckOrRegen %s --match-full-lines
// RUN: %hermesc -hermes-parser -dump-ir %s -O

function sink(a) { }

function simple_for_in_loop(obj) {
  var prop = 0;
  for (prop in obj) { sink(prop) }
}

function for_in_loop_with_break_continue(obj) {
  var prop = 0;
  for (prop in obj) { sink(prop); break; continue; }
}

function for_in_loop_with_named_break(obj) {
  var prop = 0;
goto1:
  for (prop in obj) { sink(prop); break goto1; }
}

function check_var_decl_for_in_loop(obj) {
  for (var prop in obj) { sink(prop) }
}

function loop_member_expr_lhs() {
  var x = {};

  for (x.y in [1,2,3]) { sink(x.y); }

  sink(x.y);
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:       DeclareGlobalVarInst "sink": string
// CHECK-NEXT:       DeclareGlobalVarInst "simple_for_in_loop": string
// CHECK-NEXT:       DeclareGlobalVarInst "for_in_loop_with_break_continue": string
// CHECK-NEXT:       DeclareGlobalVarInst "for_in_loop_with_named_break": string
// CHECK-NEXT:       DeclareGlobalVarInst "check_var_decl_for_in_loop": string
// CHECK-NEXT:       DeclareGlobalVarInst "loop_member_expr_lhs": string
// CHECK-NEXT:  %7 = CreateFunctionInst (:object) %0: environment, %VS0: any, %sink(): functionCode
// CHECK-NEXT:       StorePropertyLooseInst %7: object, globalObject: object, "sink": string
// CHECK-NEXT:  %9 = CreateFunctionInst (:object) %0: environment, %VS0: any, %simple_for_in_loop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %9: object, globalObject: object, "simple_for_in_loop": string
// CHECK-NEXT:  %11 = CreateFunctionInst (:object) %0: environment, %VS0: any, %for_in_loop_with_break_continue(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %11: object, globalObject: object, "for_in_loop_with_break_continue": string
// CHECK-NEXT:  %13 = CreateFunctionInst (:object) %0: environment, %VS0: any, %for_in_loop_with_named_break(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %13: object, globalObject: object, "for_in_loop_with_named_break": string
// CHECK-NEXT:  %15 = CreateFunctionInst (:object) %0: environment, %VS0: any, %check_var_decl_for_in_loop(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %15: object, globalObject: object, "check_var_decl_for_in_loop": string
// CHECK-NEXT:  %17 = CreateFunctionInst (:object) %0: environment, %VS0: any, %loop_member_expr_lhs(): functionCode
// CHECK-NEXT:        StorePropertyLooseInst %17: object, globalObject: object, "loop_member_expr_lhs": string
// CHECK-NEXT:  %19 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:        StoreStackInst undefined: undefined, %19: any
// CHECK-NEXT:  %21 = LoadStackInst (:any) %19: any
// CHECK-NEXT:        ReturnInst %21: any
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

// CHECK:scope %VS2 [obj: any, prop: any]

// CHECK:function simple_for_in_loop(obj: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS2: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS2.obj]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS2.prop]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS2.prop]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %8 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %9 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS2.obj]: any
// CHECK-NEXT:        StoreStackInst %10: any, %7: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %6: any, %7: any, %8: number, %9: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %12: any, %7: any, %8: number, %9: number, %6: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %16: any, [%VS2.prop]: any
// CHECK-NEXT:  %18 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %19 = LoadFrameInst (:any) %1: environment, [%VS2.prop]: any
// CHECK-NEXT:  %20 = CallInst (:any) %18: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %19: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:scope %VS3 [obj: any, prop: any]

// CHECK:function for_in_loop_with_break_continue(obj: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS3: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS3.obj]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS3.prop]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS3.prop]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %8 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %9 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS3.obj]: any
// CHECK-NEXT:        StoreStackInst %10: any, %7: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %6: any, %7: any, %8: number, %9: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %12: any, %7: any, %8: number, %9: number, %6: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %16 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %16: any, [%VS3.prop]: any
// CHECK-NEXT:  %18 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %19 = LoadFrameInst (:any) %1: environment, [%VS3.prop]: any
// CHECK-NEXT:  %20 = CallInst (:any) %18: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %19: any
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:function_end

// CHECK:scope %VS4 [obj: any, prop: any]

// CHECK:function for_in_loop_with_named_break(obj: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS4: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS4.obj]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS4.prop]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, 0: number, [%VS4.prop]: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %7 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %8 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %9 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %10 = LoadFrameInst (:any) %1: environment, [%VS4.obj]: any
// CHECK-NEXT:        StoreStackInst %10: any, %7: any
// CHECK-NEXT:  %12 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %6: any, %7: any, %8: number, %9: number, %BB2, %BB3
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        BranchInst %BB1
// CHECK-NEXT:%BB3:
// CHECK-NEXT:        GetNextPNameInst %12: any, %7: any, %8: number, %9: number, %6: any, %BB2, %BB4
// CHECK-NEXT:%BB4:
// CHECK-NEXT:  %17 = LoadStackInst (:any) %12: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %17: any, [%VS4.prop]: any
// CHECK-NEXT:  %19 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [%VS4.prop]: any
// CHECK-NEXT:  %21 = CallInst (:any) %19: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %20: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:scope %VS5 [obj: any, prop: any]

// CHECK:function check_var_decl_for_in_loop(obj: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS5: any, %0: environment
// CHECK-NEXT:  %2 = LoadParamInst (:any) %obj: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %2: any, [%VS5.obj]: any
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS5.prop]: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %7 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %8 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %9 = LoadFrameInst (:any) %1: environment, [%VS5.obj]: any
// CHECK-NEXT:        StoreStackInst %9: any, %6: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %5: any, %6: any, %7: number, %8: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %11: any, %6: any, %7: number, %8: number, %5: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %15 = LoadStackInst (:any) %11: any
// CHECK-NEXT:        StoreFrameInst %1: environment, %15: any, [%VS5.prop]: any
// CHECK-NEXT:  %17 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %18 = LoadFrameInst (:any) %1: environment, [%VS5.prop]: any
// CHECK-NEXT:  %19 = CallInst (:any) %17: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %18: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end

// CHECK:scope %VS6 [x: any]

// CHECK:function loop_member_expr_lhs(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  %0 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  %1 = CreateScopeInst (:environment) %VS6: any, %0: environment
// CHECK-NEXT:       StoreFrameInst %1: environment, undefined: undefined, [%VS6.x]: any
// CHECK-NEXT:  %3 = AllocObjectLiteralInst (:object) empty: any
// CHECK-NEXT:       StoreFrameInst %1: environment, %3: object, [%VS6.x]: any
// CHECK-NEXT:  %5 = AllocStackInst (:any) $?anon_0_iter: any
// CHECK-NEXT:  %6 = AllocStackInst (:any) $?anon_1_base: any
// CHECK-NEXT:  %7 = AllocStackInst (:number) $?anon_2_idx: any
// CHECK-NEXT:  %8 = AllocStackInst (:number) $?anon_3_size: any
// CHECK-NEXT:  %9 = AllocArrayInst (:object) 3: number, 1: number, 2: number, 3: number
// CHECK-NEXT:        StoreStackInst %9: object, %6: any
// CHECK-NEXT:  %11 = AllocStackInst (:any) $?anon_4_prop: any
// CHECK-NEXT:        GetPNamesInst %5: any, %6: any, %7: number, %8: number, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  %13 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %14 = LoadFrameInst (:any) %1: environment, [%VS6.x]: any
// CHECK-NEXT:  %15 = LoadPropertyInst (:any) %14: any, "y": string
// CHECK-NEXT:  %16 = CallInst (:any) %13: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %15: any
// CHECK-NEXT:        ReturnInst undefined: undefined
// CHECK-NEXT:%BB2:
// CHECK-NEXT:        GetNextPNameInst %11: any, %6: any, %7: number, %8: number, %5: any, %BB1, %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  %19 = LoadStackInst (:any) %11: any
// CHECK-NEXT:  %20 = LoadFrameInst (:any) %1: environment, [%VS6.x]: any
// CHECK-NEXT:        StorePropertyLooseInst %19: any, %20: any, "y": string
// CHECK-NEXT:  %22 = LoadPropertyInst (:any) globalObject: object, "sink": string
// CHECK-NEXT:  %23 = LoadFrameInst (:any) %1: environment, [%VS6.x]: any
// CHECK-NEXT:  %24 = LoadPropertyInst (:any) %23: any, "y": string
// CHECK-NEXT:  %25 = CallInst (:any) %22: any, empty: any, false: boolean, empty: any, undefined: undefined, undefined: undefined, %24: any
// CHECK-NEXT:        BranchInst %BB2
// CHECK-NEXT:function_end
