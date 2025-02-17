/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -dump-ir %s | %FileCheckOrRegen --check-prefix=CHKIR --match-full-lines %s
// RUN: %shermes -dump-lir %s | %FileCheckOrRegen --check-prefix=CHKLIR --match-full-lines %s

function foo(i) {
  switch (i) {
    case 0:
      i = 2;
    case 1:
      break;
  }
  return i;
}

// Auto-generated content below. Please do not modify manually.

// CHKIR:scope %VS0 []

// CHKIR:function global(): undefined
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHKIR-NEXT:       DeclareGlobalVarInst "foo": string
// CHKIR-NEXT:  %2 = CreateFunctionInst (:object) %0: environment, %foo(): functionCode
// CHKIR-NEXT:       StorePropertyLooseInst %2: object, globalObject: object, "foo": string
// CHKIR-NEXT:       ReturnInst undefined: undefined
// CHKIR-NEXT:function_end

// CHKIR:function foo(i: any): any
// CHKIR-NEXT:%BB0:
// CHKIR-NEXT:  %0 = LoadParamInst (:any) %i: any
// CHKIR-NEXT:       SwitchInst %0: any, %BB1, 0: number, %BB2, 1: number, %BB1
// CHKIR-NEXT:%BB1:
// CHKIR-NEXT:  %2 = PhiInst (:any) 2: number, %BB2, %0: any, %BB0
// CHKIR-NEXT:       ReturnInst %2: any
// CHKIR-NEXT:%BB2:
// CHKIR-NEXT:       BranchInst %BB1
// CHKIR-NEXT:function_end

// CHKLIR:scope %VS0 []

// CHKLIR:function global(): undefined
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:       DeclareGlobalVarInst "foo": string
// CHKLIR-NEXT:  %1 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHKLIR-NEXT:  %2 = CreateFunctionInst (:object) %1: environment, %foo(): functionCode
// CHKLIR-NEXT:  %3 = HBCGetGlobalObjectInst (:object)
// CHKLIR-NEXT:       StorePropertyLooseInst %2: object, %3: object, "foo": string
// CHKLIR-NEXT:  %5 = HBCLoadConstInst (:undefined) undefined: undefined
// CHKLIR-NEXT:       ReturnInst %5: undefined
// CHKLIR-NEXT:function_end

// CHKLIR:function foo(i: any): any
// CHKLIR-NEXT:%BB0:
// CHKLIR-NEXT:  %0 = LoadParamInst (:any) %i: any
// CHKLIR-NEXT:       BranchInst %BB4
// CHKLIR-NEXT:%BB1:
// CHKLIR-NEXT:  %2 = PhiInst (:any) %4: number, %BB2, %0: any, %BB3
// CHKLIR-NEXT:       ReturnInst %2: any
// CHKLIR-NEXT:%BB2:
// CHKLIR-NEXT:  %4 = HBCLoadConstInst (:number) 2: number
// CHKLIR-NEXT:       BranchInst %BB1
// CHKLIR-NEXT:%BB3:
// CHKLIR-NEXT:  %6 = HBCLoadConstInst (:number) 1: number
// CHKLIR-NEXT:  %7 = BinaryStrictlyEqualInst (:boolean) %6: number, %0: any
// CHKLIR-NEXT:       CondBranchInst %7: boolean, %BB1, %BB1
// CHKLIR-NEXT:%BB4:
// CHKLIR-NEXT:  %9 = HBCLoadConstInst (:number) 0: number
// CHKLIR-NEXT:  %10 = BinaryStrictlyEqualInst (:boolean) %9: number, %0: any
// CHKLIR-NEXT:        CondBranchInst %10: boolean, %BB2, %BB3
// CHKLIR-NEXT:function_end
