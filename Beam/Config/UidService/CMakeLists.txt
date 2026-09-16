file(GLOB_RECURSE header_files ${BEAM_INCLUDE_PATH}/Beam/UidServiceTests/*.hpp)
file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/UidServiceTests/*.cpp)
add_executable(UidServiceTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(UidServiceTests
  debug ${SQLITE_LIBRARY_DEBUG_PATH}
  optimized ${SQLITE_LIBRARY_OPTIMIZED_PATH})
target_link_libraries(UidServiceTests BeamRoutines)
add_custom_command(TARGET UidServiceTests POST_BUILD COMMAND UidServiceTests)
beam_install_target(UidServiceTests "${TEST_INSTALL_DIRECTORY}")
