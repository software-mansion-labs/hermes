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
  var numIter = 3000;
  var len = 10000;
  var a = Array(len);
  for (var i = 0; i < len; i++) {
    a[i] = i;
  }

  function add(prevSum, value) {
    return prevSum + value;
  }

  for (var i = 0; i < numIter; i++) {
    a.reduce(add, 0);
  }

  print('done');
})();
var end = new Date();
print("Time: " + (end - start));

