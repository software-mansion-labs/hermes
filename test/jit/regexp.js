/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -fno-inline -Xforce-jit -Xjit-crash-on-error %s | %FileCheck --match-full-lines %s
// REQUIRES: jit

function foo() {
  return /abc/g;
}

print(foo());
// CHECK: /abc/g
