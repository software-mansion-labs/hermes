# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

cmake_minimum_required(VERSION 3.26)
project(sh-benchmarks)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/libgui)
add_subdirectory(raytracer/typed)
add_subdirectory(imgui-demo)
add_subdirectory(sqlite-read)
if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/facebook)
    add_subdirectory(facebook)
endif()
