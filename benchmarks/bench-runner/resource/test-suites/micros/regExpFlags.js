/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 */

var start = new Date();
(function() {
  var numIter = 3000000;
  var rx = / /gimuy;

  for (var i = 0; i < numIter; i++) {
    rx.flags;
  }

  print('done');
})();
var end = new Date();
print("Time: " + (end - start));

