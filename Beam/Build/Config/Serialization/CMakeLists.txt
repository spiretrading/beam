file(GLOB header_files
  ./../../../Include/Beam/Serialization/*.hpp)
file(GLOB source_files
  ./../../../Source/Serialization/*.cpp)

add_library(Serialization ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Serialization PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME beam_serialization)
install(TARGETS Serialization CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS Serialization CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

file(GLOB test_header_files
  ./../../../Include/Beam/SerializationTests/*.hpp)
file(GLOB test_source_files
  ./../../../Source/SerializationTests/*.cpp)

add_library(SerializationTests ${test_header_files} ${test_source_files})
set_source_files_properties(${test_header_files}
  PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(SerializationTests PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME beam_serialization_tests)
install(TARGETS SerializationTests CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS SerializationTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

add_executable(SerializationTester
  ./../../../Source/SerializationTester/main.cpp)
target_link_libraries(SerializationTester SerializationTests IOTests
  debug $ENV{CPPUNIT_LIBRARY_DEBUG_PATH}
  optimized $ENV{CPPUNIT_LIBRARY_OPTIMIZED_PATH})

add_custom_command(TARGET SerializationTester POST_BUILD
  COMMAND SerializationTester)
install(TARGETS SerializationTester CONFIGURATIONS Debug
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS SerializationTester CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Release)
