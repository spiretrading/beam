file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/QueuesTests/*.cpp)
add_executable(QueuesTests ${header_files} ${source_files})
if(MSVC)
  target_precompile_headers(QueuesTests PRIVATE <doctest/doctest.h>
    <Beam/Pointers/Pointers.hpp> <Beam/Queues/Queues.hpp>
    <Beam/Routines/Routines.hpp>)
  set_source_files_properties(${BEAM_SOURCE_PATH}/QueuesTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()
if(UNIX)
  target_link_libraries(QueuesTests
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
add_custom_command(TARGET QueuesTests POST_BUILD COMMAND QueuesTests)
install(TARGETS QueuesTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS QueuesTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
