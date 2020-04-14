file(GLOB source_files ${BEAM_SOURCE_PATH}/ParsersTests/*.cpp)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

add_executable(ParsersTests ${header_files} ${source_files})

add_custom_command(TARGET ParsersTests POST_BUILD COMMAND ParsersTests)
install(TARGETS ParsersTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS ParsersTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
