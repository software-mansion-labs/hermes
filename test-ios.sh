# cmake -S . -B ./build/ios -DHERMES_EXTRA_LINKER_FLAGS="" \
#   -DHERMES_APPLE_TARGET_PLATFORM:STRING="iphonesimulator" \
#   -DCMAKE_OSX_ARCHITECTURES:STRING="arm64" \
#   -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING="15.1" \
#   -DHERMES_ENABLE_DEBUGGER:BOOLEAN=true \
#   -DHERMES_ENABLE_INTL:BOOLEAN=true \
#   -DHERMES_ENABLE_LIBFUZZER:BOOLEAN=false \
#   -DHERMES_ENABLE_FUZZILLI:BOOLEAN=false \
#   -DHERMES_ENABLE_TEST_SUITE:BOOLEAN=false \
#   -DHERMES_ENABLE_BITCODE:BOOLEAN=false \
#   -DHERMES_BUILD_APPLE_FRAMEWORK:BOOLEAN=true \
#   -DHERMES_BUILD_SHARED_JSI:BOOLEAN=false \
#   -DHERMES_BUILD_APPLE_DSYM:BOOLEAN=true \
#   -DIMPORT_HERMESC:PATH="hermesc_build" \
#   -DJSI_DIR="/Users/piaskowyk/project/react-native/packages/react-native/ReactCommon/jsi" \
#   -DHERMES_RELEASE_VERSION="new hermes for RN open source" \
#   -DCMAKE_BUILD_TYPE="Debug"

# ./hermes -O -emit-binary -out test.js.hbc test.js
# ./hermes test.js.hbc

# przerobić pliki:
# /Users/piaskowyk/sandbox/hermestest/ios/Pods/hermes-engine/destroot/include/hermes/Publiflagsc/CtorConfig.h
# /Users/piaskowyk/sandbox/hermestest/ios/Pods/hermes-engine/destroot/include/hermes/Public/RuntimeConfig.h

# rm -r destroot build_host_hermesc build_static_hermes build_iphonesimulator .cache
# debug
export DEBUG=true && ./utils/build-ios-framework.sh
cmake -S . -B build_static_hermes -G Ninja
cmake --build ./build_static_hermes -j 10
mkdir destroot/bin
cp build_static_hermes/bin/{hermes,hermesc,hermes-lit} ./destroot/bin

# release
export DEBUG=false && ./utils/build-ios-framework.sh
cmake -S . -B build_static_hermes -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build ./build_static_hermes --config Release -j 10
mkdir destroot/bin
cp build_static_hermes/bin/{hermes,hermesc,hermes-lit} ./destroot/bin