file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/NetworkTests/*.cpp)
add_executable(NetworkTests ${source_files})
target_link_libraries(NetworkTests BeamRoutines)
add_custom_command(TARGET NetworkTests POST_BUILD COMMAND NetworkTests)
beam_install_target(NetworkTests "${TEST_INSTALL_DIRECTORY}")
file(GLOB_RECURSE
  functional_source_files ${BEAM_SOURCE_PATH}/NetworkFunctionalTests/*.cpp)
add_executable(NetworkFunctionalTests ${functional_source_files})
target_link_libraries(NetworkFunctionalTests
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH})
target_link_libraries(NetworkFunctionalTests BeamRoutines)
if(WIN32)
  target_link_libraries(NetworkFunctionalTests crypt32)
endif()
beam_install_target(NetworkFunctionalTests "${TEST_INSTALL_DIRECTORY}")
