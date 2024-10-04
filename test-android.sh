# rm -rf build_android
# cmake -S . -B build_android -G "Ninja" \
#   -DHERMES_ENABLE_DEBUGGER:BOOLEAN=true \
#   -DHERMES_ENABLE_DEBUGGER=true \
#   -DHERMES_ENABLE_INTL:BOOLEAN=true \
#   -DHERMES_ENABLE_LIBFUZZER:BOOLEAN=false \
#   -DHERMES_ENABLE_FUZZILLI:BOOLEAN=false \
#   -DHERMES_ENABLE_TEST_SUITE:BOOLEAN=false \
#   -DANDROID_ABI=arm64-v8a \
#   -DHERMES_UNICODE_LITE=ON \
#   -DICU_FOUND=1 \
#   -DHERMES_FACEBOOK_BUILD=OFF \
#   -DIMPORT_HOST_COMPILERS="$PWD/build_host_hermesc/ImportHostCompilers.cmake" \
#   -DIMPORT_HERMESC:PATH="$PWD/build_host_hermesc/ImportHermesc.cmake" \
#   -DIMPORT_HERMESC="$PWD/build_host_hermesc/ImportHostCompilers.cmake" \
#   -DCMAKE_TOOLCHAIN_FILE=/Users/piaskowyk/Library/Android/sdk/ndk/26.1.10909125/build/cmake/android.toolchain.cmake \
#   -DCMAKE_BUILD_TYPE="Debug"

# cmake --build build_android -j 10

#######################################################

# rm -rf build build_android
# export HERMES_WS_DIR=$(pwd)
# cmake -S . -B build
# cmake --build ./build --target hermesc -j 10
# cd android && ./gradlew githubRelease

#######################################################

cmake . \
  -B ./build_android \
  -DHERMES_FACEBOOK_BUILD=OFF \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=/Users/piaskowyk/Library/Android/sdk/ndk/26.1.10909125/build/cmake/android.toolchain.cmake \
  -DHERMES_UNICODE_LITE=ON -DICU_FOUND=1 \
  -DIMPORT_HOST_COMPILERS="$PWD/build_host_hermesc/ImportHostCompilers.cmake" \
  -DHERMES_ENABLE_DEBUGGER=OFF \
  -DANDROID_ABI=arm64-v8a -G Ninja

cmake --build build_android -j 10