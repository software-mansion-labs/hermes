/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: (%shermes -exec %s 2>&1 || true) | %FileCheck --match-full-lines %s
"use strict";

Error.prepareStackTrace = (e, callSites) => "hello"

// Uncaught errors are formatted with prepareStackTrace
throw new Error('foo');
// CHECK: Uncaught hello
// CHECK-EMPTY
