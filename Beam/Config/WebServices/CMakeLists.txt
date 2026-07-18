file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/WebServicesTests/*.cpp)
add_executable(WebServicesTests ${source_files})
if(MSVC)
  target_precompile_headers(WebServicesTests PRIVATE <doctest/doctest.h>
    <Beam/IO/IO.hpp> <Beam/Pointers/Pointers.hpp>
    <Beam/SerializationTests/SerializationTests.hpp> <Beam/Sql/Sql.hpp>
    <Beam/Utilities/Utilities.hpp> <Beam/WebServices/WebServices.hpp>)
  set_source_files_properties(${BEAM_SOURCE_PATH}/WebServicesTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()
target_link_libraries(WebServicesTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${SQLITE_LIBRARY_DEBUG_PATH}
  optimized ${SQLITE_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug ${ZLIB_LIBRARY_DEBUG_PATH}
  optimized ${ZLIB_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(WebServicesTests
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
  target_link_libraries(WebServicesTests Bcrypt crypt32 Secur32.Lib shlwapi)
endif()
add_custom_command(TARGET WebServicesTests POST_BUILD COMMAND WebServicesTests)
install(TARGETS WebServicesTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS WebServicesTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
