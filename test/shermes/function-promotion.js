/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %shermes -O0 -exec %s | %FileCheck --match-full-lines %s
// RUN: %shermes -O -exec %s | %FileCheck --match-full-lines %s

function foo(p) {
  let x = 1;
  var y = 2;
  {
    function p() {}
    function x() {}
    function y() {}
    function z() {}
    {
      function a() {}
    }
    try { throw null } catch {
      {
        function b() {}
      }
    }
    try {
      throw null;
    } catch (c) {
        {
          function c() { return 123; }
        }
    }
  }
  // p is not promoted.
  // x is not promoted.
  // y is promoted.
  // z is promoted.
  // a is promoted.
  // b is promoted.
  // c is promoted.
  print(typeof p, typeof x, typeof y, typeof z, typeof a, typeof b, typeof c);
}

foo();
// CHECK: undefined number function function function function function
