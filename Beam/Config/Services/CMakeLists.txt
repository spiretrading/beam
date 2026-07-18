file(GLOB_RECURSE header_files ${BEAM_INCLUDE_PATH}/Beam/ServicesTests/*.hpp)
file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/ServicesTests/*.cpp)
add_executable(ServicesTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(MSVC)
  target_precompile_headers(ServicesTests PRIVATE <doctest/doctest.h>
    <Beam/Codecs/Codecs.hpp> <Beam/CodecsTests/CodecsTests.hpp>
    <Beam/Collections/Collections.hpp> <Beam/IO/IO.hpp>
    <Beam/Pointers/Pointers.hpp> <Beam/Queues/Queues.hpp>
    <Beam/Routines/Routines.hpp> <Beam/Serialization/Serialization.hpp>
    <Beam/SerializationTests/SerializationTests.hpp>
    <Beam/ServiceLocator/ServiceLocator.hpp> <Beam/Services/Services.hpp>
    <Beam/ServicesTests/ServicesTests.hpp>
    <Beam/SignalHandling/SignalHandling.hpp> <Beam/TimeService/TimeService.hpp>)
  set_source_files_properties(${BEAM_SOURCE_PATH}/ServicesTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
  target_link_libraries(ServicesTests
    debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
    debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH} crypt32)
endif()
if(UNIX)
  target_link_libraries(ServicesTests
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
  target_link_libraries(ServicesTests rt socket nsl)
endif()
add_custom_command(TARGET ServicesTests POST_BUILD COMMAND ServicesTests)
install(TARGETS ServicesTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS ServicesTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
