/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermesc -O0 -dump-lra %s | %FileCheckOrRegen --match-full-lines %s
// RUN: %hermesc -O -dump-lra %s | %FileCheckOrRegen --match-full-lines  --check-prefix=CHKOPT %s

// Check that literals are uniqued when optimizations is disabled, but aren't
// when it is enabled.

var a, b;

function foo(x) {
  if (x)
    a = 10;
  else
    b = 10;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "a": string
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "b": string
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "foo": string
// CHECK-NEXT:  $Reg1 = CreateFunctionInst (:object) $Reg1, %foo(): functionCode
// CHECK-NEXT:  $Reg2 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg1, $Reg2, "foo": string
// CHECK-NEXT:  $Reg1 = AllocStackInst (:any) $?anon_0_ret: any
// CHECK-NEXT:  $Reg2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg1 = MovInst (:undefined) $Reg2
// CHECK-NEXT:  $Reg1 = LoadStackInst (:any) $Reg1
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg1
// CHECK-NEXT:function_end

// CHECK:scope %VS0 []

// CHECK:scope %VS1 [x: any]

// CHECK:function foo(x: any): any
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg1 = GetParentScopeInst (:environment) %VS0: any, %parentScope: environment
// CHECK-NEXT:  $Reg1 = CreateScopeInst (:environment) %VS1: any, $Reg1
// CHECK-NEXT:  $Reg2 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  $Reg0 = StoreFrameInst $Reg1, $Reg2, [%VS1.x]: any
// CHECK-NEXT:  $Reg1 = LoadFrameInst (:any) $Reg1, [%VS1.x]: any
// CHECK-NEXT:  $Reg0 = CondBranchInst $Reg1, %BB1, %BB2
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg1 = HBCLoadConstInst (:number) 10: number
// CHECK-NEXT:  $Reg2 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg1, $Reg2, "a": string
// CHECK-NEXT:  $Reg0 = BranchInst %BB3
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg1 = HBCLoadConstInst (:number) 10: number
// CHECK-NEXT:  $Reg2 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg1, $Reg2, "b": string
// CHECK-NEXT:  $Reg0 = BranchInst %BB3
// CHECK-NEXT:%BB3:
// CHECK-NEXT:  $Reg1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg1
// CHECK-NEXT:function_end

// CHKOPT:scope %VS0 []

// CHKOPT:function global(): undefined
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  $Reg1 = DeclareGlobalVarInst "a": string
// CHKOPT-NEXT:  $Reg1 = DeclareGlobalVarInst "b": string
// CHKOPT-NEXT:  $Reg1 = DeclareGlobalVarInst "foo": string
// CHKOPT-NEXT:  $Reg1 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHKOPT-NEXT:  $Reg0 = CreateFunctionInst (:object) $Reg1, %foo(): functionCode
// CHKOPT-NEXT:  $Reg1 = HBCGetGlobalObjectInst (:object)
// CHKOPT-NEXT:  $Reg1 = StorePropertyLooseInst $Reg0, $Reg1, "foo": string
// CHKOPT-NEXT:  $Reg1 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKOPT-NEXT:  $Reg1 = ReturnInst $Reg1
// CHKOPT-NEXT:function_end

// CHKOPT:function foo(x: any): undefined
// CHKOPT-NEXT:%BB0:
// CHKOPT-NEXT:  $Reg0 = HBCLoadConstInst (:number) 10: number
// CHKOPT-NEXT:  $Reg2 = HBCGetGlobalObjectInst (:object)
// CHKOPT-NEXT:  $Reg1 = LoadParamInst (:any) %x: any
// CHKOPT-NEXT:  $Reg1 = CondBranchInst $Reg1, %BB1, %BB2
// CHKOPT-NEXT:%BB1:
// CHKOPT-NEXT:  $Reg2 = StorePropertyLooseInst $Reg0, $Reg2, "a": string
// CHKOPT-NEXT:  $Reg2 = BranchInst %BB3
// CHKOPT-NEXT:%BB2:
// CHKOPT-NEXT:  $Reg1 = StorePropertyLooseInst $Reg0, $Reg2, "b": string
// CHKOPT-NEXT:  $Reg1 = BranchInst %BB3
// CHKOPT-NEXT:%BB3:
// CHKOPT-NEXT:  $Reg2 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKOPT-NEXT:  $Reg2 = ReturnInst $Reg2
// CHKOPT-NEXT:function_end
