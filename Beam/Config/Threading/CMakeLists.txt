file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/ThreadingTests/*.cpp)
add_executable(ThreadingTests ${header_files} ${source_files})
beam_configure_target(ThreadingTests DEPENDENCIES CRYPTOPP)
target_link_libraries(ThreadingTests PUBLIC BeamRoutines)
add_custom_command(TARGET ThreadingTests
  POST_BUILD COMMAND ThreadingTests)
beam_install_target(ThreadingTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
file(GLOB_RECURSE
  functional_source_files ${BEAM_SOURCE_PATH}/ThreadingFunctionalTests/*.cpp)
add_executable(ThreadingFunctionalTests ${functional_source_files})
beam_configure_target(ThreadingFunctionalTests DEPENDENCIES CRYPTOPP)
target_link_libraries(ThreadingFunctionalTests PUBLIC BeamRoutines)
beam_install_target(
  ThreadingFunctionalTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
