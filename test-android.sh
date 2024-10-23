rm -rf build_android

cmake . \
  -B ./build_android \
  -DHERMES_FACEBOOK_BUILD=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=/Users/piaskowyk/Library/Android/sdk/ndk/26.1.10909125/build/cmake/android.toolchain.cmake \
  -DHERMES_ENABLE_DEBUGGER:BOOLEAN=true \
  -DHERMES_ENABLE_DEBUGGER=true \
  -DIMPORT_HOST_COMPILERS="$PWD/build_host_hermes/ImportHostCompilers.cmake" \
  -DHERMES_ENABLE_DEBUGGER=ON \
  -DANDROID_ABI=arm64-v8a \
  -DHERMESVM_ALLOW_JIT=2 \
  -DHERMESVM_ALLOW_JIT:STRING=2 \
  -DHERMES_ENABLE_INTL=true \
  -DHERMES_ENABLE_INTL:BOOLEAN=true \
  -DHERMES_IS_ANDROID=True \
  -DCMAKE_CXX_FLAGS_RELEASE="-O3 -DNDEBUG" \
  -DANDROID_ABI=arm64-v8a \
  -G Ninja
  # -DHERMESVM_ALLOW_JIT=2 \
  # -DHERMESVM_ALLOW_JIT:STRING=2 \

cmake --build build_android -j 10

# export HERMES_WS_DIR=$(pwd)
# cmake -S . -B build
# cmake --build ./build --target hermesc -j 10
# cd android && ./gradlew githubRelease