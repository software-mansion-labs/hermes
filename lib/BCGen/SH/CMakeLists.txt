# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_hermes_library(hermesSHBackend
  LoweringPasses.h
  LowerCalls.cpp
  LoadConstants.cpp
  RecreateCheapValues.cpp
  LowerNanBoxedUnionNarrowT.cpp
  PeepholeLowering.cpp
  SH.cpp
  SHRegAlloc.cpp SHRegAlloc.h
  LineDirectiveEmitter.cpp LineDirectiveEmitter.h
  LINK_OBJLIBS
  hermesBackend
  hermesInst
  hermesSourceMap
  hermesAST
)
