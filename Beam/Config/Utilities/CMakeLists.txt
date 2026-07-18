file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/UtilitiesTests/*.cpp)
add_executable(UtilitiesTests ${source_files})
if(MSVC)
  target_precompile_headers(UtilitiesTests PRIVATE <doctest/doctest.h>
    <Beam/SerializationTests/SerializationTests.hpp>
    <Beam/Utilities/Utilities.hpp>)
  set_source_files_properties(${BEAM_SOURCE_PATH}/UtilitiesTests/main.cpp
    PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif()
target_compile_definitions(UtilitiesTests PRIVATE YAML_CPP_STATIC_DEFINE)
target_link_libraries(UtilitiesTests
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(UtilitiesTests
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
add_custom_command(TARGET UtilitiesTests POST_BUILD COMMAND UtilitiesTests)
install(TARGETS UtilitiesTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS UtilitiesTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
