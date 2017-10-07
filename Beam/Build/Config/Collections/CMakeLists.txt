file(GLOB header_files
  ./../../../Include/Beam/Collections/*.hpp)
file(GLOB source_files
  ./../../../Source/Collections/*.cpp)

add_library(Collections ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(Collections PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME beam_collections)
install(TARGETS Collections CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS Collections CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

file(GLOB test_header_files
  ./../../../Include/Beam/CollectionsTests/*.hpp)
file(GLOB test_source_files
  ./../../../Source/CollectionsTests/*.cpp)

add_library(CollectionsTests ${test_header_files} ${test_source_files})
set_source_files_properties(${test_header_files}
  PROPERTIES HEADER_FILE_ONLY TRUE)
set_target_properties(CollectionsTests PROPERTIES
  STATIC_LIBRARY_FLAGS_RELEASE "${CMAKE_LIBRARY_FLAGS}"
  OUTPUT_NAME beam_collections_tests)
install(TARGETS CollectionsTests CONFIGURATIONS Debug
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Debug)
install(TARGETS CollectionsTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{LIB_INSTALL_DIRECTORY}/Release)

add_executable(CollectionsTester
  ./../../../Source/CollectionsTester/main.cpp)
target_link_libraries(CollectionsTester Collections CollectionsTests
  debug $ENV{CPPUNIT_LIBRARY_DEBUG_PATH}
  optimized $ENV{CPPUNIT_LIBRARY_OPTIMIZED_PATH}
  debug $ENV{CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized $ENV{CRYPTOPP_LIBRARY_OPTIMIZED_PATH})

if(UNIX)
  target_link_libraries(CollectionsTester
    debug $ENV{BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_REGEX_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_REGEX_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    debug $ENV{BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized $ENV{BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    pthread rt)
endif()

add_custom_command(TARGET CollectionsTester POST_BUILD
  COMMAND CollectionsTester)
install(TARGETS CollectionsTester CONFIGURATIONS Debug
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS CollectionsTester CONFIGURATIONS Release RelWithDebInfo
  DESTINATION $ENV{TEST_INSTALL_DIRECTORY}/Release)
