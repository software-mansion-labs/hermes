/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O0 -target=HBC -dump-bytecode %s | %FileCheckOrRegen --match-full-lines %s

// We're checking that the "a[3]" results in a GetByIndex bytecode.

function foo(a) {
    return a[3];
}

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 2
// CHECK-NEXT:  String count: 2
// CHECK-NEXT:  BigInt count: 0
// CHECK-NEXT:  String Kind Entry count: 2
// CHECK-NEXT:  RegExp count: 0
// CHECK-NEXT:  Segment ID: 0
// CHECK-NEXT:  CommonJS module count: 0
// CHECK-NEXT:  CommonJS module count (static): 0
// CHECK-NEXT:  Function source count: 0
// CHECK-NEXT:  Bytecode options:
// CHECK-NEXT:    staticBuiltins: 0
// CHECK-NEXT:    cjsModulesStaticallyResolved: 0

// CHECK:Global String Table:
// CHECK-NEXT:s0[ASCII, 0..5]: global
// CHECK-NEXT:i1[ASCII, 6..8] #9290584E: foo

// CHECK:Function<global>(1 params, 3 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    CreateTopLevelEnvironment r1, 0
// CHECK-NEXT:    DeclareGlobalVar  "foo"
// CHECK-NEXT:    CreateClosure     r1, r1, Function<foo>
// CHECK-NEXT:    GetGlobalObject   r2
// CHECK-NEXT:    PutByIdLoose      r2, r1, 1, "foo"
// CHECK-NEXT:    LoadConstUndefined r2
// CHECK-NEXT:    Mov               r1, r2
// CHECK-NEXT:    Ret               r1

// CHECK:Function<foo>(2 params, 3 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x000a, lexical 0x0000
// CHECK-NEXT:    GetParentEnvironment r1, 0
// CHECK-NEXT:    CreateEnvironment r1, r1, 1
// CHECK-NEXT:    LoadParam         r2, 1
// CHECK-NEXT:    StoreToEnvironment r1, 0, r2
// CHECK-NEXT:    LoadFromEnvironment r1, r1, 0
// CHECK-NEXT:    GetByIndex        r1, r1, 3
// CHECK-NEXT:    Ret               r1

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}get-by-int-const.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 12 col 1
// CHECK-NEXT:    bc 6: line 12 col 1
// CHECK-NEXT:    bc 18: line 12 col 1
// CHECK-NEXT:  0x000a  function idx 1, starts at line 12 col 1
// CHECK-NEXT:    bc 21: line 13 col 13
// CHECK-NEXT:  0x0011  end of debug source table
