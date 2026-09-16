file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/WebServicesTests/*.cpp)
add_executable(WebServicesTests ${source_files})
target_link_libraries(WebServicesTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${SQLITE_LIBRARY_DEBUG_PATH}
  optimized ${SQLITE_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug ${ZLIB_LIBRARY_DEBUG_PATH}
  optimized ${ZLIB_LIBRARY_OPTIMIZED_PATH})
target_link_libraries(WebServicesTests BeamRoutines)
if(WIN32)
  target_link_libraries(WebServicesTests Bcrypt crypt32 Secur32.Lib shlwapi)
endif()
add_custom_command(TARGET WebServicesTests POST_BUILD COMMAND WebServicesTests)
beam_install_target(WebServicesTests "${TEST_INSTALL_DIRECTORY}")
