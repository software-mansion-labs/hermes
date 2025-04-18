/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (! %hermesc -parse-flow -dump-ast -pretty-json %s 2>&1) | %FileCheck %s --match-full-lines

type T = {| x: number, ... |};
// CHECK: {{.*}}:10:10: error: Explicit inexact syntax cannot appear inside an explicit exact object type
// CHECK: type T = {| x: number, ... |};
// CHECK:          ^
