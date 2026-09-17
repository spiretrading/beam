file(GLOB_RECURSE
  header_files ${BEAM_INCLUDE_PATH}/Beam/ServiceLocatorTests/*.hpp)
file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/ServiceLocatorTests/*.cpp)
add_executable(ServiceLocatorTests ${header_files} ${source_files})
beam_configure_target(ServiceLocatorTests DEPENDENCIES
  CRYPTOPP SQLITE OPEN_SSL OPEN_SSL_BASE YAML)
target_compile_definitions(ServiceLocatorTests PRIVATE YAML_CPP_STATIC_DEFINE)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(ServiceLocatorTests PUBLIC BeamRoutines)
if(WIN32)
  target_link_libraries(ServiceLocatorTests PUBLIC
    Bcrypt crypt32 Secur32.Lib shlwapi)
endif()
add_custom_command(TARGET ServiceLocatorTests
  POST_BUILD COMMAND ServiceLocatorTests)
beam_install_target(ServiceLocatorTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
