file(GLOB header_files
  ./../../../Include/Beam/Reactors/*.hpp)
file(GLOB source_files
  ./../../../Source/Reactors/*.cpp)

add_library(Reactors ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Reactors PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME beam_reactors)
install(TARGETS Reactors CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS Reactors CONFIGURATIONS Release
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

file(GLOB test_header_files
  ./../../../Include/Beam/ReactorsTests/*.hpp)
file(GLOB test_source_files
  ./../../../Source/ReactorsTests/*.cpp)

add_library(ReactorsTests ${test_header_files} ${test_source_files})
set_source_files_properties(${test_header_files}
  PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(ReactorsTests PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME beam_reactors_tests)
install(TARGETS ReactorsTests CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS ReactorsTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

add_executable(ReactorsTester
  ./../../../Source/ReactorsTester/main.cpp)
target_link_libraries(ReactorsTester ReactorsTests
  debug $ENV{CPPUNIT_LIBRARY_DEBUG_PATH}
  optimized $ENV{CPPUNIT_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{LUA_LIBRARY_DEBUG_PATH}
  optimized $ENV{LUA_LIBRARY_OPTIMIZED_PATH})

if(UNIX)
  target_link_libraries(ReactorsTester
    debug $ENV{BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_REGEX_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_REGEX_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    dl pthread rt)
endif(UNIX)

add_custom_command(TARGET ReactorsTester POST_BUILD COMMAND ReactorsTester)
install(TARGETS ReactorsTester CONFIGURATIONS Debug
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS ReactorsTester CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Release)
