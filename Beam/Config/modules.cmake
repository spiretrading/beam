include_directories(${BEAM_MODULES_PATH})
include_directories(SYSTEM ${ASPEN_INCLUDE_PATH})
include_directories(SYSTEM ${CRYPTOPP_INCLUDE_PATH})
include_directories(SYSTEM ${DOCTEST_INCLUDE_PATH})
include_directories(SYSTEM ${OPEN_SSL_INCLUDE_PATH})
include_directories(SYSTEM ${VIPER_INCLUDE_PATH})
include_directories(SYSTEM ${ZLIB_INCLUDE_PATH})
file(GLOB aspen_module_files "${ASPEN_MODULES_PATH}/*.ixx")
add_library(aspen STATIC ${aspen_module_files})
set_target_properties(aspen PROPERTIES LINKER_LANGUAGE CXX OUTPUT_NAME aspen)
file(GLOB_RECURSE beam_module_files "${BEAM_MODULES_PATH}/*.ixx"
  "${BEAM_MODULES_PATH}/*.cpp")
list(FILTER beam_module_files EXCLUDE REGEX "/Python/")
add_library(Beam STATIC ${beam_module_files})
set_target_properties(Beam PROPERTIES LINKER_LANGUAGE CXX OUTPUT_NAME Beam)
if(MSVC)
  target_compile_options(Beam PRIVATE $<$<CONFIG:Release>:/GL->)
endif()
target_link_libraries(Beam aspen
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug ${ZLIB_LIBRARY_DEBUG_PATH}
  optimized ${ZLIB_LIBRARY_OPTIMIZED_PATH})
if(WIN32)
  target_link_libraries(Beam bcrypt crypt32 Secur32 shlwapi Ws2_32)
endif()
