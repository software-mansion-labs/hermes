/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (echo "START" && %hermesc -dump-ir -w %s 2>&1 >/dev/null && echo "END" ) | %FileCheck %s --match-full-lines
// REQUIRES: !fbcode_coverage

// CHECK: START
"use strict";
print(missing_global);

// CHECK-NEXT: END
