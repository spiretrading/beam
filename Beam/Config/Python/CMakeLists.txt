file(GLOB_RECURSE header_files ${BEAM_INCLUDE_PATH}/Beam/Python/*.hpp)
file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/Python/*.cpp)
include_directories(SYSTEM ${PYBIND11_INCLUDE_PATH})
include_directories(SYSTEM ${PYTHON_INCLUDE_PATH})
add_library(Python SHARED ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Python PROPERTIES OUTPUT_NAME beam)
target_compile_definitions(
  Python PRIVATE ASPEN_USE_DLL BEAM_BUILD_DLL BEAM_ENABLE_STACK_PRINT
  YAML_CPP_STATIC_DEFINE)
set_target_properties(Python PROPERTIES CXX_VISIBILITY_PRESET default
  VISIBILITY_INLINES_HIDDEN 0)
if(MSVC)
  target_compile_definitions(Python PRIVATE HAVE_SNPRINTF)
  set_target_properties(Python PROPERTIES SUFFIX ".pyd")
endif()
target_compile_options(Python PRIVATE
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-fvisibility=hidden>)
if(UNIX)
  set_target_properties(Python PROPERTIES PREFIX "")
  if(APPLE)
    set_target_properties(Python PROPERTIES
      SUFFIX ".so" INSTALL_RPATH "@loader_path")
  else()
    set_target_properties(Python PROPERTIES INSTALL_RPATH "$ORIGIN")
  endif()
endif()
target_link_libraries(Python
  debug ${ASPEN_PYTHON_LIBRARY_DEBUG_PATH}
  optimized ${ASPEN_PYTHON_LIBRARY_OPTIMIZED_PATH}
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${MYSQL_LIBRARY_DEBUG_PATH}
  optimized ${MYSQL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug ${PYTHON_LIBRARY_DEBUG_PATH}
  optimized ${PYTHON_LIBRARY_OPTIMIZED_PATH}
  debug ${SQLITE_LIBRARY_DEBUG_PATH}
  optimized ${SQLITE_LIBRARY_OPTIMIZED_PATH}
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH}
  debug ${ZLIB_LIBRARY_DEBUG_PATH}
  optimized ${ZLIB_LIBRARY_OPTIMIZED_PATH})
target_link_libraries(Python BeamRoutines)
if(UNIX AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_link_libraries(Python debug stdc++exp)
endif()
if(WIN32)
  target_link_libraries(Python bcrypt crypt32 Secur32 shlwapi Ws2_32)
endif()
beam_install_target(Python "${LIB_INSTALL_DIRECTORY}")
