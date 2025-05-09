/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %shermes -exec %s 2>&1 ) | %FileCheck --match-full-lines %s

(() => {
  try {
    var {} = {};
  } catch (e) {
    print(e.message);
  }
})();

throw new Error("Foo");

// CHECK-LABEL: Uncaught Error: Foo
