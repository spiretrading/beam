file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/SignalHandlingTests/*.cpp)
add_executable(SignalHandlingTests ${source_files})
target_link_libraries(SignalHandlingTests
  debug ${SQLITE_LIBRARY_DEBUG_PATH}
  optimized ${SQLITE_LIBRARY_OPTIMIZED_PATH})
target_link_libraries(SignalHandlingTests BeamRoutines)
add_custom_command(
  TARGET SignalHandlingTests POST_BUILD COMMAND SignalHandlingTests)
beam_install_target(SignalHandlingTests "${TEST_INSTALL_DIRECTORY}")
