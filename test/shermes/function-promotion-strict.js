/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -O -exec %s | %FileCheck --match-full-lines %s

function foo(p) {
  'use strict';
  let x = 1;
  var y = 2;
  {
    function p() {}
    function x() {}
    function y() {}
    function z() {}
  }
  // Nothing is promoted.
  print(typeof p, typeof x, typeof y, typeof z);
}

foo();
// CHECK: undefined number number undefined
