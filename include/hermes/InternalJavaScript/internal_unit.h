/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef HERMES_INTERNALJAVASCRIPT_INTERNALUNIT_H
#define HERMES_INTERNALJAVASCRIPT_INTERNALUNIT_H

struct SHUnit;

/// Instantiate a pre-compiled SHUnit to be included with the VM upon
/// construction. This module must be run before any user code can be run.
extern "C" struct SHUnit *sh_export_internal_unit();

#endif // HERMES_INTERNALJAVASCRIPT_INTERNALUNIT_H
