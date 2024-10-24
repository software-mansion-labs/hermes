/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -target=HBC -dump-bytecode -O %s | %FileCheckOrRegen --match-full-lines %s

var obj = {
  get b() {},
  set b(x) {},
  get c() {},
  set d(x) {},
};

// Auto-generated content below. Please do not modify manually.

// CHECK:Bytecode File Information:
// CHECK-NEXT:  Bytecode version number: {{.*}}
// CHECK-NEXT:  Source hash: {{.*}}
// CHECK-NEXT:  Function count: 5
// CHECK-NEXT:  String count: 9
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
// CHECK-NEXT:s0[ASCII, 0..4]: get b
// CHECK-NEXT:s1[ASCII, 4..4]: b
// CHECK-NEXT:s2[ASCII, 5..9]: get c
// CHECK-NEXT:s3[ASCII, 9..9]: c
// CHECK-NEXT:s4[ASCII, 10..14]: set d
// CHECK-NEXT:s5[ASCII, 14..14]: d
// CHECK-NEXT:s6[ASCII, 15..20]: global
// CHECK-NEXT:s7[ASCII, 21..25]: set b
// CHECK-NEXT:i8[ASCII, 26..28] #DC53DBCF: obj

// CHECK:Function<global>(1 params, 5 registers, 0 numbers, 0 non-pointers):
// CHECK-NEXT:Offset in debug table: source 0x0000, lexical 0x0000
// CHECK-NEXT:    CreateTopLevelEnvironment r3, 0
// CHECK-NEXT:    DeclareGlobalVar  "obj"
// CHECK-NEXT:    NewObject         r2
// CHECK-NEXT:    CreateClosure     r0, r3, Function<get b>
// CHECK-NEXT:    CreateClosure     r1, r3, Function<set b>
// CHECK-NEXT:    LoadConstString   r4, "b"
// CHECK-NEXT:    PutOwnGetterSetterByVal r2, r4, r0, r1, 1
// CHECK-NEXT:    CreateClosure     r0, r3, Function<get c>
// CHECK-NEXT:    LoadConstUndefined r4
// CHECK-NEXT:    LoadConstString   r1, "c"
// CHECK-NEXT:    PutOwnGetterSetterByVal r2, r1, r0, r4, 1
// CHECK-NEXT:    CreateClosure     r1, r3, Function<set d>
// CHECK-NEXT:    LoadConstString   r3, "d"
// CHECK-NEXT:    PutOwnGetterSetterByVal r2, r3, r4, r1, 1
// CHECK-NEXT:    GetGlobalObject   r3
// CHECK-NEXT:    PutByIdLoose      r3, r2, 1, "obj"
// CHECK-NEXT:    Ret               r4

// CHECK:Function<get b>(1 params, 1 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<set b>(2 params, 1 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<get c>(1 params, 1 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Function<set d>(2 params, 1 registers, 0 numbers, 1 non-pointers):
// CHECK-NEXT:    LoadConstUndefined r0
// CHECK-NEXT:    Ret               r0

// CHECK:Debug filename table:
// CHECK-NEXT:  0: {{.*}}gettersetter.js

// CHECK:Debug file table:
// CHECK-NEXT:  source table offset 0x0000: filename id 0

// CHECK:Debug source table:
// CHECK-NEXT:  0x0000  function idx 0, starts at line 10 col 1
// CHECK-NEXT:    bc 6: line 10 col 1
// CHECK-NEXT:    bc 27: line 10 col 11
// CHECK-NEXT:    bc 44: line 10 col 11
// CHECK-NEXT:    bc 59: line 10 col 11
// CHECK-NEXT:    bc 67: line 10 col 9
// CHECK-NEXT:  0x0013  end of debug source table
