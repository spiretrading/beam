file(GLOB_RECURSE header_files ${BEAM_INCLUDE_PATH}/Beam/UidServiceTests/*.hpp)
file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/UidServiceTests/*.cpp)
add_executable(UidServiceTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
if(MSVC)
  target_precompile_headers(UidServiceTests PRIVATE <doctest/doctest.h>
    <Beam/IO/IO.hpp> <Beam/Serialization/Serialization.hpp>
    <Beam/Services/Services.hpp> <Beam/ServicesTests/ServicesTests.hpp>
    <Beam/Sql/Sql.hpp> <Beam/TimeService/TimeService.hpp>
    <Beam/UidService/UidService.hpp> <Beam/UidServiceTests/UidServiceTests.hpp>)
  set_source_files_properties(${BEAM_SOURCE_PATH}/UidServiceTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
  target_link_libraries(UidServiceTests
    debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
    debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
    optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH} crypt32)
endif()
target_link_libraries(UidServiceTests
  debug ${SQLITE_LIBRARY_DEBUG_PATH}
  optimized ${SQLITE_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(UidServiceTests
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
add_custom_command(TARGET UidServiceTests POST_BUILD COMMAND UidServiceTests)
install(TARGETS UidServiceTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS UidServiceTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
