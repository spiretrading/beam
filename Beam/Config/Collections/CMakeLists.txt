file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/CollectionsTests/*.cpp)
add_executable(CollectionsTests ${source_files})
if(MSVC)
  target_precompile_headers(CollectionsTests PRIVATE <doctest/doctest.h>
    <Beam/Collections/Collections.hpp>
    <Beam/SerializationTests/SerializationTests.hpp>
    <Beam/Utilities/Utilities.hpp>)
  set_source_files_properties(${BEAM_SOURCE_PATH}/CollectionsTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()
if(UNIX)
  target_link_libraries(CollectionsTests
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
add_custom_command(TARGET CollectionsTests POST_BUILD COMMAND CollectionsTests)
install(TARGETS CollectionsTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS CollectionsTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
