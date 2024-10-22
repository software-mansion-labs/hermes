rm -rf ./build_host_hermes
cmake -S . -B build_host_hermes \
  -DCMAKE_BUILD_TYPE=Release \
  -DHERMESVM_ALLOW_JIT=2 \
  -DHERMESVM_ALLOW_JIT:STRING=2 \
  -G Ninja
cmake --build ./build_host_hermes -j 10