file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/RoutinesTests/*.cpp)
add_executable(RoutinesTests ${source_files})
target_link_libraries(RoutinesTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH})
target_link_libraries(RoutinesTests BeamRoutines)
add_custom_command(TARGET RoutinesTests POST_BUILD COMMAND RoutinesTests)
beam_install_target(RoutinesTests "${TEST_INSTALL_DIRECTORY}")
