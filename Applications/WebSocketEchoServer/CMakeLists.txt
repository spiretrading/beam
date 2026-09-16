cmake_minimum_required(VERSION 3.28)
project(WebSocketEchoServer LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(D "${CMAKE_BINARY_DIR}/Dependencies" CACHE STRING
  "Path to dependencies folder.")
file(TO_NATIVE_PATH "${D}" D)
set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE
    STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
if(CMAKE_CONFIGURATION_TYPES AND
    NOT "MinSizeRel" IN_LIST CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES};MinSizeRel"
    CACHE STRING "Available build configurations." FORCE)
endif()
if(WIN32)
  set(configure_script
    cmd /c CALL "${CMAKE_SOURCE_DIR}/configure.bat" -DD "${D}"
    "${CMAKE_BUILD_TYPE}")
elseif(UNIX)
  set(configure_script "${CMAKE_SOURCE_DIR}/configure.sh" "-DD=${D}"
    "${CMAKE_BUILD_TYPE}")
endif()
execute_process(
  COMMAND "${CMAKE_COMMAND}" -E env "BEAM_SKIP_CMAKE=1" ${configure_script}
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}" RESULT_VARIABLE configure_result
  OUTPUT_VARIABLE configure_output ERROR_VARIABLE configure_error
  OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)
if(NOT configure_result EQUAL 0)
  message(FATAL_ERROR "Configuration script failed with error:\n"
    "${configure_error}\nOutput:\n${configure_output}")
endif()
include(../../Beam/Config/dependencies.cmake)
include(../../Beam/Config/targets.cmake)
include_directories(${BEAM_INCLUDE_PATH})
include_directories(SYSTEM ${BOOST_INCLUDE_PATH})
include_directories(SYSTEM ${CRYPTOPP_INCLUDE_PATH})
include_directories(SYSTEM ${OPEN_SSL_INCLUDE_PATH})
include_directories(SYSTEM ${TCLAP_INCLUDE_PATH})
include_directories(SYSTEM ${YAML_INCLUDE_PATH})
include_directories(SYSTEM ${ZLIB_INCLUDE_PATH})
link_directories(${BOOST_DEBUG_PATH})
link_directories(${BOOST_OPTIMIZED_PATH})
if(MSVC)
  add_compile_options(/bigobj /external:anglebrackets /external:W0
    $<$<CONFIG:Release>:/GL> /MP /WX /Zc:__cplusplus /Zc:preprocessor)
  add_compile_definitions(_CRT_SECURE_NO_DEPRECATE NOMINMAX
    _SCL_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN _WIN32_WINNT=0x0A00)
  add_link_options($<$<CONFIG:Release>:/LTCG>)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  add_compile_options(-g $<$<CONFIG:Release>:-DNDEBUG>)
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    add_compile_options(-fsized-deallocation)
  endif()
endif()
if(CYGWIN)
  add_compile_definitions(__USE_W32_SOCKETS)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "SunOS")
  add_compile_options($<$<CONFIG:Release>:-pthreads>)
endif()
include_directories(Include)
include_directories("${PROJECT_BINARY_DIR}")
file(GLOB_RECURSE header_files Include/WebSocketEchoServer/*.hpp)
list(APPEND header_files "${PROJECT_BINARY_DIR}/Version.hpp")
file(GLOB_RECURSE source_files Source/*.cpp)
add_executable(WebSocketEchoServer ${header_files} ${source_files})
target_compile_definitions(WebSocketEchoServer PRIVATE YAML_CPP_STATIC_DEFINE)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(WebSocketEchoServer
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH}
  debug ${ZLIB_LIBRARY_DEBUG_PATH}
  optimized ${ZLIB_LIBRARY_OPTIMIZED_PATH})
target_link_libraries(WebSocketEchoServer BeamRoutines)
if(WIN32)
  target_link_libraries(WebSocketEchoServer Crypt32.lib)
endif()
install(TARGETS WebSocketEchoServer
  DESTINATION "${PROJECT_BINARY_DIR}/Application")
install(FILES Application/config.default.yml Application/setup.py
  ../Python/setup_utils.py DESTINATION "${PROJECT_BINARY_DIR}/Application")
if(UNIX)
  install(PROGRAMS Application/check.sh Application/start.sh
    Application/stop.sh DESTINATION "${PROJECT_BINARY_DIR}/Application")
endif()
set_property(GLOBAL APPEND PROPERTY BEAM_BUILD_TARGETS WebSocketEchoServer)
set_property(GLOBAL APPEND PROPERTY BEAM_INSTALLED_OUTPUTS
  "${PROJECT_BINARY_DIR}/Application/$<TARGET_FILE_NAME:${PROJECT_NAME}>")
beam_configure_clean()
