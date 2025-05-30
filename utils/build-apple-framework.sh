#!/bin/bash
# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if [ "$DEBUG" = true ]; then
  BUILD_TYPE="Debug"
else
  BUILD_TYPE="Release"
fi

function command_exists {
  command -v "${1}" > /dev/null 2>&1
}

if command_exists "cmake"; then
  if command_exists "ninja"; then
    BUILD_SYSTEM="Ninja"
  else
    BUILD_SYSTEM="Unix Makefiles"
  fi
else
  echo >&2 'CMake is required to install Hermes, install it with: brew install cmake'
  exit 1
fi

function get_release_version {
  ruby -rcocoapods-core -rjson -e "puts Pod::Specification.from_file('hermes-engine.podspec').version"
}

function get_ios_deployment_target {
  ruby -rcocoapods-core -rjson -e "puts Pod::Specification.from_file('hermes-engine.podspec').deployment_target('ios')"
}

function get_visionos_deployment_target {
  ruby -rcocoapods-core -rjson -e "puts Pod::Specification.from_file('hermes-engine.podspec').deployment_target('visionos')"
}

function get_tvos_deployment_target {
  ruby -rcocoapods-core -rjson -e "puts Pod::Specification.from_file('hermes-engine.podspec').deployment_target('tvos')"
}

function get_mac_deployment_target {
  ruby -rcocoapods-core -rjson -e "puts Pod::Specification.from_file('hermes-engine.podspec').deployment_target('osx')"
}

# Build host hermes compiler for internal bytecode
function build_host_hermesc {
  cmake -S . -B build_host_hermesc
  cmake --build ./build_host_hermesc
}

# Utility function to configure an Apple framework
function configure_apple_framework {
  local build_cli_tools
  if [[ $1 == macosx ]]; then
    build_cli_tools="true"
  else
    build_cli_tools="false"
  fi

  cmake -S . -B "build_$1" -G "$BUILD_SYSTEM" \
    -DHERMES_APPLE_TARGET_PLATFORM:STRING="$1" \
    -DCMAKE_OSX_ARCHITECTURES:STRING="$2" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING="$3" \
    -DHERMES_ENABLE_DEBUGGER:BOOLEAN=true \
    -DHERMES_ENABLE_INTL:BOOLEAN=true \
    -DHERMES_ENABLE_LIBFUZZER:BOOLEAN=false \
    -DHERMES_ENABLE_FUZZILLI:BOOLEAN=false \
    -DHERMES_ENABLE_TEST_SUITE:BOOLEAN=false \
    -DHERMES_BUILD_APPLE_FRAMEWORK:BOOLEAN=true \
    -DCMAKE_CXX_FLAGS="-gdwarf" \
    -DCMAKE_C_FLAGS="-gdwarf" \
    -DHERMES_ENABLE_TOOLS:BOOLEAN="$build_cli_tools" \
    -DIMPORT_HERMESC:PATH="$PWD/build_host_hermesc/ImportHermesc.cmake" \
    -DCMAKE_INSTALL_PREFIX:PATH=../destroot \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_CXX_FLAGS="-gdwarf" \
    -DCMAKE_C_FLAGS="-gdwarf" \
    -DIMPORT_HOST_COMPILERS="$PWD/build_host_hermesc/ImportHostCompilers.cmake"
}

function generate_dSYM {
  TARGET_PLATFORM="$1"
  DSYM_PATH="$PWD/build_$TARGET_PLATFORM/lib/hermesvm.framework.dSYM"
  xcrun dsymutil "$PWD/build_$TARGET_PLATFORM/lib/hermesvm.framework/hermesvm" --out "$DSYM_PATH"
  mkdir -p "$PWD/destroot/Library/Frameworks/$TARGET_PLATFORM"
  cp -R "$DSYM_PATH" "$PWD/destroot/Library/Frameworks/$TARGET_PLATFORM"
}

# Utility function to build an Apple framework
function build_apple_framework {
  echo "Building framework for $1 with architectures: $2"

  build_host_hermesc
  [ ! -f "$PWD/build_host_hermesc/ImportHermesc.cmake" ] &&
  echo "Host hermesc is required to build apple frameworks!"

  configure_apple_framework "$1" "$2" "$3"

  if [[ "$BUILD_SYSTEM" == "Ninja" ]]; then
    (cd "./build_$1" && ninja install/strip)
  else
    (cd "./build_$1" && make install/strip)
  fi

  generate_dSYM "$1"
}

# Accepts an array of frameworks and will place all of
# the architectures into an universal folder and then remove
# the merged frameworks from destroot
function create_universal_framework {
  mkdir destroot/bin
  cp build_host_hermesc/bin/{hermes,hermesc,hermes-lit} ./destroot/bin

  cd ./destroot/Library/Frameworks || exit 1

  local platforms=("$@")
  local args=""

  echo "Creating universal framework for platforms: ${platforms[*]}"

  for i in "${!platforms[@]}"; do
    args+="-framework ${platforms[$i]}/hermesvm.framework "
  done

  mkdir universal
  xcodebuild -create-xcframework $args -output "universal/hermesvm.xcframework"

  for platform in $@; do
    rm -r "$platform"
  done

  cd - || exit 1
}
