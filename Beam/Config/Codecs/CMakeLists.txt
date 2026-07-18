file(GLOB_RECURSE header_files ${BEAM_INCLUDE_PATH}/Beam/CodecsTests/*.hpp)
file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/CodecsTests/*.cpp)
add_executable(CodecsTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(MSVC)
  target_precompile_headers(CodecsTests PRIVATE <doctest/doctest.h>
    <Beam/Codecs/Codecs.hpp> <Beam/CodecsTests/CodecsTests.hpp>
    <Beam/IO/IO.hpp> <Beam/Utilities/Utilities.hpp>)
  set_source_files_properties(${BEAM_SOURCE_PATH}/CodecsTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()
target_link_libraries(CodecsTests
  debug ${ZLIB_LIBRARY_DEBUG_PATH}
  optimized ${ZLIB_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(CodecsTests
    debug ${BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    pthread rt)
endif()
add_custom_command(TARGET CodecsTests POST_BUILD COMMAND CodecsTests)
install(TARGETS CodecsTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS CodecsTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
