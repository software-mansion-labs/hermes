/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

// RUN: %hermes -O -target=HBC %s
// RUN: %shermes -exec %s

function Err() {}
Err.prototype = Error()
var s = new Err()
s.stack = "default"
