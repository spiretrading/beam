file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/PointersTests/*.cpp)
add_executable(PointersTests ${header_files} ${source_files})
add_custom_command(TARGET PointersTests POST_BUILD COMMAND PointersTests)
install(TARGETS PointersTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS PointersTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
