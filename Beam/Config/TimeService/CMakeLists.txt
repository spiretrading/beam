file(GLOB header_files ${BEAM_INCLUDE_PATH}/Beam/TimeServiceTests/*.hpp)
file(GLOB source_files ${BEAM_SOURCE_PATH}/TimeServiceTests/*.cpp)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
endif()

add_executable(TimeServiceTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(TimeServiceTests
  debug ${CPPUNIT_LIBRARY_DEBUG_PATH}
  optimized ${CPPUNIT_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH})

if(UNIX)
  target_link_libraries(TimeServiceTests
    debug ${BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    dl pthread rt)
endif()
if(WIN32)
  target_link_libraries(TimeServiceTests Crypt32.lib)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL SunOS)
  target_link_libraries(TimeServiceTests rt socket nsl)
endif()

add_custom_command(TARGET TimeServiceTests POST_BUILD COMMAND TimeServiceTests)
install(TARGETS TimeServiceTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS TimeServiceTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
