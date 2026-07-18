file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/ThreadingTests/*.cpp)
add_executable(ThreadingTests ${header_files} ${source_files})
if(MSVC)
  target_precompile_headers(ThreadingTests PRIVATE <doctest/doctest.h>
    <Beam/Threading/Threading.hpp>)
  set_source_files_properties(${BEAM_SOURCE_PATH}/ThreadingTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()
target_link_libraries(ThreadingTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(ThreadingTests
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
add_custom_command(TARGET ThreadingTests
  POST_BUILD COMMAND ThreadingTests)
install(TARGETS ThreadingTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS ThreadingTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
