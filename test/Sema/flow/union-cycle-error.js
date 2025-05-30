/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -Werror -typed -dump-sema %s 2>&1 ) | %FileCheck --match-full-lines %s

type A = A | A;

// CHECK:{{.*}}:10:1: error: ft: type contains a circular reference to itself
// CHECK-NEXT:type A = A | A;
// CHECK-NEXT:^~~~~~~~~~~~~~~
