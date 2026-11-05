file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/NetworkTests/*.cpp)
add_executable(NetworkTests ${source_files})
beam_configure_target(NetworkTests)
target_link_libraries(NetworkTests PUBLIC BeamRoutines)
add_custom_command(TARGET NetworkTests POST_BUILD COMMAND NetworkTests)
beam_install_target(NetworkTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
file(GLOB_RECURSE
  functional_source_files ${BEAM_SOURCE_PATH}/NetworkFunctionalTests/*.cpp)
add_executable(NetworkFunctionalTests ${functional_source_files})
beam_configure_target(NetworkFunctionalTests DEPENDENCIES
  CRYPTOPP OPEN_SSL OPEN_SSL_BASE YAML)
target_compile_definitions(
  NetworkFunctionalTests PRIVATE YAML_CPP_STATIC_DEFINE)
target_link_libraries(NetworkFunctionalTests PUBLIC BeamRoutines)
if(WIN32)
  target_link_libraries(NetworkFunctionalTests PUBLIC crypt32)
endif()
beam_install_target(
  NetworkFunctionalTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
