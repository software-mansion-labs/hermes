/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes %s | %FileCheck %s --match-full-lines
// RUN: %shermes -exec %s | %FileCheck %s --match-full-lines
// XFAIL: *

var obj = {
  ['asdf']: function() {}
}

print(obj.asdf.name);
// CHECK: asdf
