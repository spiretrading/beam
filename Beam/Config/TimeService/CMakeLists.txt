file(GLOB_RECURSE
  header_files ${BEAM_INCLUDE_PATH}/Beam/TimeServiceTests/*.hpp)
file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/TimeServiceTests/*.cpp)
add_executable(TimeServiceTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(MSVC)
  target_precompile_headers(TimeServiceTests PRIVATE <doctest/doctest.h>
    <Beam/Collections/Collections.hpp> <Beam/IO/IO.hpp>
    <Beam/Pointers/Pointers.hpp> <Beam/Queues/Queues.hpp>
    <Beam/Routines/Routines.hpp> <Beam/Threading/Threading.hpp>
    <Beam/TimeService/TimeService.hpp>
    <Beam/TimeServiceTests/TimeServiceTests.hpp>)
  set_source_files_properties(${BEAM_SOURCE_PATH}/TimeServiceTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()
target_link_libraries(TimeServiceTests
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH})
if(WIN32)
  target_link_libraries(TimeServiceTests crypt32)
endif()
if(UNIX)
  target_link_libraries(TimeServiceTests
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
if(${CMAKE_SYSTEM_NAME} STREQUAL SunOS)
  target_link_libraries(TimeServiceTests rt socket nsl)
endif()
add_custom_command(TARGET TimeServiceTests POST_BUILD COMMAND TimeServiceTests)
install(TARGETS TimeServiceTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS TimeServiceTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
