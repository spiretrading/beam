file(GLOB_RECURSE header_files ${BEAM_INCLUDE_PATH}/Beam/Python/*.hpp)
file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/Python/*.cpp)
include_directories(SYSTEM ${PYBIND11_INCLUDE_PATH})
include_directories(SYSTEM ${PYTHON_INCLUDE_PATH})
file(GLOB aspen_dll_module_files "${ASPEN_MODULES_PATH}/*.ixx")
add_library(aspen_dll STATIC ${aspen_dll_module_files})
set_target_properties(aspen_dll PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(aspen_dll PRIVATE ASPEN_USE_DLL)
file(GLOB aspen_python_module_files "${ASPEN_MODULES_PATH}/Python/*.ixx")
add_library(aspen_python STATIC ${aspen_python_module_files})
set_target_properties(aspen_python PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(aspen_python PRIVATE ASPEN_USE_DLL)
target_link_libraries(aspen_python aspen_dll)
file(GLOB beam_module_files "${BEAM_MODULES_PATH}/*.ixx")
add_library(BeamModules STATIC ${beam_module_files})
set_target_properties(BeamModules PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(BeamModules PRIVATE ASPEN_USE_DLL BEAM_USE_DLL
  BEAM_ENABLE_STACK_PRINT)
target_link_libraries(BeamModules aspen_dll)
file(GLOB beam_python_module_files "${BEAM_MODULES_PATH}/Python/*.ixx")
add_library(BeamPython STATIC ${beam_python_module_files})
set_target_properties(BeamPython PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(BeamPython PRIVATE ASPEN_USE_DLL BEAM_USE_DLL
  BEAM_ENABLE_STACK_PRINT)
target_link_libraries(BeamPython aspen_dll aspen_python)
add_library(Python SHARED ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Python PROPERTIES OUTPUT_NAME beam)
target_compile_definitions(
  Python PRIVATE ASPEN_USE_DLL BEAM_BUILD_DLL BEAM_ENABLE_STACK_PRINT
  YAML_CPP_STATIC_DEFINE)
set_target_properties(Python PROPERTIES CXX_VISIBILITY_PRESET default
  VISIBILITY_INLINES_HIDDEN 0)
if(MSVC)
  target_compile_definitions(aspen_dll PRIVATE HAVE_SNPRINTF)
  target_compile_definitions(aspen_python PRIVATE HAVE_SNPRINTF)
  target_compile_definitions(BeamModules PRIVATE HAVE_SNPRINTF)
  target_compile_definitions(BeamPython PRIVATE HAVE_SNPRINTF)
  target_compile_definitions(Python PRIVATE HAVE_SNPRINTF)
  set_target_properties(Python PROPERTIES SUFFIX ".pyd")
endif()
target_compile_options(Python PRIVATE
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-fvisibility=hidden>)
if(UNIX)
  set_target_properties(Python PROPERTIES PREFIX "" INSTALL_RPATH "$ORIGIN")
endif()
target_link_libraries(Python BeamModules aspen_dll aspen_python
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
if(UNIX)
  target_link_libraries(Python
    debug ${BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_THREAD_LIBRARY_OPTIMIZED_PATH})
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(Python debug stdc++exp)
  endif()
endif()
if(WIN32)
  target_link_libraries(Python bcrypt crypt32 Secur32 shlwapi Ws2_32)
endif()
install(TARGETS Python CONFIGURATIONS Debug
  DESTINATION ${LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS Python CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${LIB_INSTALL_DIRECTORY}/Release)
