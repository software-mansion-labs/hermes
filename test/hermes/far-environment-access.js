/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (%hermesc -O0 -target=HBC %s --dump-bytecode 2>&1 || true) | %FileCheck --match-full-lines %s
// RUN: (%hermes -O0 -target=HBC %s 2>&1 || true) | %FileCheck --match-full-lines %s
// DISABLED: %hermes -O0 -target=HBC %s -lazy 2>&1 | %FileCheck --match-full-lines %s --check-prefix=LAZY
// XFAIL: windows
// UNSUPPORTED: ubsan

// Hermes has a 256 limit on the number of scopes nesting. This test ensures
// that the compiler will handle applications that require more scopes
// gracefully -- i.e., it won't crash, nor emit bad bytecode.

"use strict";

function sink(x) { return x; }
function foo(){
  var var0 = sink("1");
  var var1 = sink("2");
  var var2 = sink("3");
  return ()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=>()=> [var0, var1, var2];
}

try {
  print(foo()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()()());
} catch (e) {
  print(e);
}


// CHECK: {{.*}}far-environment-access.js:{{[0-9]+}}:{{[0-9]+}}: error: Variable environment is out-of-reach
// LAZY: SyntaxError: {{[0-9]+}}:{{[0-9]+}}:Variable environment is out-of-reach
