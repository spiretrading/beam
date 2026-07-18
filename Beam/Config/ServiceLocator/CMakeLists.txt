file(GLOB_RECURSE
  header_files ${BEAM_INCLUDE_PATH}/Beam/ServiceLocatorTests/*.hpp)
file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/ServiceLocatorTests/*.cpp)
add_executable(ServiceLocatorTests ${header_files} ${source_files})
target_compile_definitions(ServiceLocatorTests PRIVATE YAML_CPP_STATIC_DEFINE)
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(MSVC)
  target_precompile_headers(ServiceLocatorTests PRIVATE <doctest/doctest.h>
    <Beam/Codecs/Codecs.hpp> <Beam/IO/IO.hpp> <Beam/Queues/Queues.hpp>
    <Beam/Serialization/Serialization.hpp>
    <Beam/SerializationTests/SerializationTests.hpp>
    <Beam/ServiceLocator/ServiceLocator.hpp>
    <Beam/ServiceLocatorTests/ServiceLocatorTests.hpp>
    <Beam/Services/Services.hpp> <Beam/ServicesTests/ServicesTests.hpp>
    <Beam/SignalHandling/SignalHandling.hpp> <Beam/Sql/Sql.hpp>
    <Beam/TimeService/TimeService.hpp> <Beam/Utilities/Utilities.hpp>)
  set_source_files_properties(${BEAM_SOURCE_PATH}/ServiceLocatorTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()
target_link_libraries(ServiceLocatorTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${SQLITE_LIBRARY_DEBUG_PATH}
  optimized ${SQLITE_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(ServiceLocatorTests
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
if(WIN32)
  target_link_libraries(ServiceLocatorTests Bcrypt crypt32 Secur32.Lib shlwapi)
endif()
add_custom_command(TARGET ServiceLocatorTests
  POST_BUILD COMMAND ServiceLocatorTests)
install(TARGETS ServiceLocatorTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS ServiceLocatorTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
