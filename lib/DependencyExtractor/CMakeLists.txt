# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(source_files
  DependencyExtractor.cpp
)

add_hermes_library(hermesDependencyExtractor ${source_files}
  LINK_OBJLIBS
  hermesAST
  hermesParser
  hermesSupport
)
