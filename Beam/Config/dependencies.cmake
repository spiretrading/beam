if(WIN32)
  include("${CMAKE_CURRENT_LIST_DIR}/dependencies.windows.cmake")
else()
  include("${CMAKE_CURRENT_LIST_DIR}/dependencies.posix.cmake")
endif()
include("${PROJECT_BINARY_DIR}/Dependencies/aspen/Config/dependencies.cmake")
include("${PROJECT_BINARY_DIR}/Dependencies/viper/Config/dependencies.cmake")
if(UNIX)
  set(openssl_path "${PROJECT_BINARY_DIR}/Dependencies/openssl-3.6.0")
  unset(OPEN_SSL_BASE_LIBRARY_DEBUG_PATH)
  unset(OPEN_SSL_LIBRARY_DEBUG_PATH)
  find_library(OPEN_SSL_BASE_LIBRARY_DEBUG_PATH NAMES libcrypto.a
    PATHS "${openssl_path}/lib" "${openssl_path}/lib64"
    NO_DEFAULT_PATH NO_CACHE REQUIRED)
  find_library(OPEN_SSL_LIBRARY_DEBUG_PATH NAMES libssl.a
    PATHS "${openssl_path}/lib" "${openssl_path}/lib64"
    NO_DEFAULT_PATH NO_CACHE REQUIRED)
  set(OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH
    "${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}")
  set(OPEN_SSL_LIBRARY_OPTIMIZED_PATH "${OPEN_SSL_LIBRARY_DEBUG_PATH}")
endif()
set(BEAM_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/../Include")
set(BEAM_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../Source")
set(TCLAP_INCLUDE_PATH
  "${PROJECT_BINARY_DIR}/Dependencies/tclap-1.4.0-rc2/include")
