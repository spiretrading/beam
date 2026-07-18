file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/JsonTests/*.cpp)
add_executable(JsonTests ${source_files})
if(MSVC)
  target_precompile_headers(JsonTests PRIVATE <doctest/doctest.h>
    <Beam/Json/Json.hpp> <Beam/Parsers/Parsers.hpp>
    <Beam/Utilities/Utilities.hpp>)
  set_source_files_properties(${BEAM_SOURCE_PATH}/JsonTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()
if(UNIX)
  target_link_libraries(JsonTests
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
add_custom_command(TARGET JsonTests POST_BUILD COMMAND JsonTests)
install(TARGETS JsonTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS JsonTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
