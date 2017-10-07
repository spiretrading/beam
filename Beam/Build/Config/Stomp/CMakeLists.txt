file(GLOB header_files
  ./../../../Include/Beam/Stomp/*.hpp)
file(GLOB source_files
  ./../../../Source/Stomp/*.cpp)

add_library(Stomp ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Stomp PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME beam_stomp)
install(TARGETS Stomp CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS Stomp CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

file(GLOB test_header_files
  ./../../../Include/Beam/StompTests/*.hpp)
file(GLOB test_source_files
  ./../../../Source/StompTests/*.cpp)

add_library(StompTests ${test_header_files} ${test_source_files})
set_source_files_properties(${test_header_files}
  PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(StompTests PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME beam_stomp_tests)
install(TARGETS StompTests CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS StompTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

add_executable(StompTester ./../../../Source/StompTester/main.cpp)
target_link_libraries(StompTester StompTests
  debug $ENV{CPPUNIT_LIBRARY_DEBUG_PATH}
  optimized $ENV{CPPUNIT_LIBRARY_OPTIMIZED_PATH})

if(UNIX)
  target_link_libraries(StompTester
    debug $ENV{BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    pthread rt)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL SunOS)
  target_link_libraries(StompTester rt socket nsl)
endif()

add_custom_command(TARGET StompTester POST_BUILD COMMAND StompTester)
install(TARGETS StompTester CONFIGURATIONS Debug
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS StompTester CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Release)
