file(GLOB_RECURSE header_files ${BEAM_INCLUDE_PATH}/Beam/QueriesTests/*.hpp)
file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/QueriesTests/*.cpp)
add_executable(QueriesTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(QueriesTests
  debug ${SQLITE_LIBRARY_DEBUG_PATH}
  optimized ${SQLITE_LIBRARY_OPTIMIZED_PATH})
target_link_libraries(QueriesTests BeamRoutines)
add_custom_command(TARGET QueriesTests POST_BUILD COMMAND QueriesTests)
beam_install_target(QueriesTests "${TEST_INSTALL_DIRECTORY}")
