file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/UtilitiesTests/*.cpp)
add_executable(UtilitiesTests ${source_files})
target_compile_definitions(UtilitiesTests PRIVATE YAML_CPP_STATIC_DEFINE)
target_link_libraries(UtilitiesTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH})
target_link_libraries(UtilitiesTests BeamRoutines)
add_custom_command(TARGET UtilitiesTests POST_BUILD COMMAND UtilitiesTests)
beam_install_target(UtilitiesTests "${TEST_INSTALL_DIRECTORY}")
