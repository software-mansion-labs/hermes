# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

set(HERMES_ENABLE_EH ON)

add_hermes_tool(synth
  synth.cpp
)

target_link_libraries(synth
  hermesConsoleHost_obj
  hermesvm_a
  traceInterpreter_obj
  synthTrace_obj
  synthTraceParser_obj
)
