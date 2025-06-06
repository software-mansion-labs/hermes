/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @flow strict-local
 * @format
 */

export default function invariant(condition: boolean, format: string): void {
  'inline';

  if (!condition) {
    throw new Error(format);
  }
}
