file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/WebServicesTests/*.cpp)
add_executable(WebServicesTests ${source_files})
beam_configure_target(WebServicesTests DEPENDENCIES
  CRYPTOPP SQLITE OPEN_SSL OPEN_SSL_BASE ZLIB)
target_link_libraries(WebServicesTests PUBLIC BeamRoutines)
if(WIN32)
  target_link_libraries(WebServicesTests PUBLIC
    Bcrypt crypt32 Secur32.Lib shlwapi)
endif()
add_custom_command(TARGET WebServicesTests POST_BUILD COMMAND WebServicesTests)
beam_install_target(WebServicesTests "${TEST_INSTALL_DIRECTORY}/$<CONFIG>")
