file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/SignalHandlingTests/*.cpp)
add_executable(SignalHandlingTests ${source_files})
beam_configure_target(SignalHandlingTests DEPENDENCIES SQLITE)
target_link_libraries(SignalHandlingTests PUBLIC BeamRoutines)
add_custom_command(
  TARGET SignalHandlingTests POST_BUILD COMMAND SignalHandlingTests)
beam_install_target(SignalHandlingTests "${TEST_INSTALL_DIRECTORY}")
