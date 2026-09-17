file(GLOB_RECURSE
  header_files ${BEAM_INCLUDE_PATH}/Beam/SerializationTests/*.hpp)
file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/SerializationTests/*.cpp)
add_executable(SerializationTests ${header_files} ${source_files})
beam_configure_target(SerializationTests)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(SerializationTests PUBLIC BeamRoutines)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries(SerializationTests PUBLIC atomic)
endif()
add_custom_command(TARGET SerializationTests
  POST_BUILD COMMAND SerializationTests)
beam_install_target(SerializationTests "${TEST_INSTALL_DIRECTORY}")
