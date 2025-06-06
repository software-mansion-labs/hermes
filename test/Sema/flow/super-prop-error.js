/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1 ) | %FileCheckOrRegen --match-full-lines %s
// RUN: (! %hermesc -Werror -typed -dump-ir %s 2>&1 ) | %FileCheck --match-full-lines %s

class A {
}

class B extends A {
  constructor() {
    super();
  }

  f() {
    return super.x;
  }
}

// Auto-generated content below. Please do not modify manually.

// CHECK:{{.*}}super-prop-error.js:20:18: error: ft: property x not defined in class A
// CHECK-NEXT:    return super.x;
// CHECK-NEXT:                 ^
// CHECK-NEXT:Emitted 1 errors. exiting.
