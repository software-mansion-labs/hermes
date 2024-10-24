/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -dump-ra -O %s | %FileCheckOrRegen %s --match-full-lines

function main(x, y, z) {

  var sum = 3;
  for (var i = 0; i < 10; i++) {
    sum += (x + i) * (y + i);
  }

  return sum;
}

// Auto-generated content below. Please do not modify manually.

// CHECK:scope %VS0 []

// CHECK:function global(): undefined
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg0 = DeclareGlobalVarInst "main": string
// CHECK-NEXT:  $Reg0 = CreateScopeInst (:environment) %VS0: any, empty: any
// CHECK-NEXT:  $Reg1 = CreateFunctionInst (:object) $Reg0, %main(): functionCode
// CHECK-NEXT:  $Reg0 = HBCGetGlobalObjectInst (:object)
// CHECK-NEXT:  $Reg0 = StorePropertyLooseInst $Reg1, $Reg0, "main": string
// CHECK-NEXT:  $Reg0 = HBCLoadConstInst (:undefined) undefined: undefined
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end

// CHECK:function main(x: any, y: any, z: any): number
// CHECK-NEXT:%BB0:
// CHECK-NEXT:  $Reg6 = LoadParamInst (:any) %x: any
// CHECK-NEXT:  $Reg5 = LoadParamInst (:any) %y: any
// CHECK-NEXT:  $Reg4 = HBCLoadConstInst (:number) 0: number
// CHECK-NEXT:  $Reg3 = HBCLoadConstInst (:number) 3: number
// CHECK-NEXT:  $Reg2 = HBCLoadConstInst (:number) 1: number
// CHECK-NEXT:  $Reg1 = HBCLoadConstInst (:number) 10: number
// CHECK-NEXT:  $Reg4 = MovInst (:number) $Reg4
// CHECK-NEXT:  $Reg3 = MovInst (:number) $Reg3
// CHECK-NEXT:  $Reg0 = BranchInst %BB1
// CHECK-NEXT:%BB1:
// CHECK-NEXT:  $Reg4 = PhiInst (:number) $Reg4, %BB0, $Reg4, %BB1
// CHECK-NEXT:  $Reg3 = PhiInst (:number) $Reg3, %BB0, $Reg3, %BB1
// CHECK-NEXT:  $Reg7 = BinaryAddInst (:string|number) $Reg6, $Reg4
// CHECK-NEXT:  $Reg0 = BinaryAddInst (:string|number) $Reg5, $Reg4
// CHECK-NEXT:  $Reg0 = BinaryMultiplyInst (:number) $Reg7, $Reg0
// CHECK-NEXT:  $Reg0 = FAddInst (:number) $Reg3, $Reg0
// CHECK-NEXT:  $Reg7 = FAddInst (:number) $Reg4, $Reg2
// CHECK-NEXT:  $Reg3 = MovInst (:number) $Reg0
// CHECK-NEXT:  $Reg4 = MovInst (:number) $Reg7
// CHECK-NEXT:  $Reg1 = HBCFCmpBrLessThanInst $Reg4, $Reg1, %BB1, %BB2
// CHECK-NEXT:%BB2:
// CHECK-NEXT:  $Reg0 = ReturnInst $Reg0
// CHECK-NEXT:function_end
