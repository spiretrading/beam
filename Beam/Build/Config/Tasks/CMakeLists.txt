file(GLOB header_files
  ./../../../Include/Beam/Tasks/*.hpp)
file(GLOB source_files
  ./../../../Source/Tasks/*.cpp)

add_library(Tasks ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Tasks PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME beam_tasks)
install(TARGETS Tasks CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS Tasks CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

file(GLOB test_header_files
  ./../../../Include/Beam/TasksTests/*.hpp)
file(GLOB test_source_files
  ./../../../Source/TasksTests/*.cpp)

add_library(TasksTests ${test_header_files} ${test_source_files})
set_source_files_properties(${test_header_files}
  PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(TasksTests PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME beam_tasks_tests)
install(TARGETS TasksTests CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS TasksTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

add_executable(TasksTester ./../../../Source/TasksTester/main.cpp)
target_link_libraries(TasksTester TasksTests
  debug $ENV{CPPUNIT_LIBRARY_DEBUG_PATH}
  optimized $ENV{CPPUNIT_LIBRARY_OPTIMIZED_PATH})

if(UNIX)
  target_link_libraries(TasksTester
    debug $ENV{BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    pthread rt)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL SunOS)
  target_link_libraries(TasksTester rt socket nsl)
endif()

add_custom_command(TARGET TasksTester POST_BUILD
  COMMAND TasksTester)
install(TARGETS TasksTester CONFIGURATIONS Debug
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS TasksTester CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Release)
