file(GLOB header_files ${BEAM_INCLUDE_PATH}/Beam/SerializationTests/*.hpp)
file(GLOB source_files ${BEAM_SOURCE_PATH}/SerializationTests/*.cpp)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

add_executable(SerializationTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(SerializationTests
  debug ${CPPUNIT_LIBRARY_DEBUG_PATH}
  optimized ${CPPUNIT_LIBRARY_OPTIMIZED_PATH})

add_custom_command(TARGET SerializationTests POST_BUILD
  COMMAND SerializationTests)
install(TARGETS SerializationTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS SerializationTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
