file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/SqlTests/*.cpp)
add_executable(SqlTests ${source_files})
target_link_libraries(SqlTests
  debug ${SQLITE_LIBRARY_DEBUG_PATH}
  optimized ${SQLITE_LIBRARY_OPTIMIZED_PATH})
target_link_libraries(SqlTests BeamRoutines)
add_custom_command(TARGET SqlTests POST_BUILD COMMAND SqlTests)
beam_install_target(SqlTests "${TEST_INSTALL_DIRECTORY}")
