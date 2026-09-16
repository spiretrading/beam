file(GLOB_RECURSE
  header_files ${BEAM_INCLUDE_PATH}/Beam/ServiceLocatorTests/*.hpp)
file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/ServiceLocatorTests/*.cpp)
add_executable(ServiceLocatorTests ${header_files} ${source_files})
target_compile_definitions(ServiceLocatorTests PRIVATE YAML_CPP_STATIC_DEFINE)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(ServiceLocatorTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${SQLITE_LIBRARY_DEBUG_PATH}
  optimized ${SQLITE_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH})
target_link_libraries(ServiceLocatorTests BeamRoutines)
if(WIN32)
  target_link_libraries(ServiceLocatorTests Bcrypt crypt32 Secur32.Lib shlwapi)
endif()
add_custom_command(TARGET ServiceLocatorTests
  POST_BUILD COMMAND ServiceLocatorTests)
beam_install_target(ServiceLocatorTests "${TEST_INSTALL_DIRECTORY}")
