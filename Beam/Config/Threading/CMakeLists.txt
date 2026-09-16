file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/ThreadingTests/*.cpp)
add_executable(ThreadingTests ${header_files} ${source_files})
target_link_libraries(ThreadingTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH})
target_link_libraries(ThreadingTests BeamRoutines)
add_custom_command(TARGET ThreadingTests
  POST_BUILD COMMAND ThreadingTests)
beam_install_target(ThreadingTests "${TEST_INSTALL_DIRECTORY}")
