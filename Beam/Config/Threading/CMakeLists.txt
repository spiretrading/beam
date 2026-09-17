file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/ThreadingTests/*.cpp)
add_executable(ThreadingTests ${header_files} ${source_files})
beam_configure_target(ThreadingTests DEPENDENCIES CRYPTOPP)
target_link_libraries(ThreadingTests PUBLIC BeamRoutines)
add_custom_command(TARGET ThreadingTests
  POST_BUILD COMMAND ThreadingTests)
beam_install_target(ThreadingTests "${TEST_INSTALL_DIRECTORY}")
