file(GLOB_RECURSE header_files ${BEAM_INCLUDE_PATH}/Beam/QueriesTests/*.hpp)
file(GLOB_RECURSE source_files ${BEAM_SOURCE_PATH}/QueriesTests/*.cpp)
add_executable(QueriesTests ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(QueriesTests
  debug ${SQLITE_LIBRARY_DEBUG_PATH}
  optimized ${SQLITE_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(QueriesTests
    debug ${BOOST_CHRONO_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_CONTEXT_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_DATE_TIME_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_THREAD_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_THREAD_LIBRARY_OPTIMIZED_PATH}
    debug ${BOOST_SYSTEM_LIBRARY_DEBUG_PATH}
    optimized ${BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH}
    dl pthread rt)
endif()
add_custom_command(TARGET QueriesTests POST_BUILD COMMAND QueriesTests)
install(TARGETS QueriesTests CONFIGURATIONS Debug
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Debug)
install(TARGETS QueriesTests CONFIGURATIONS Release RelWithDebInfo
  DESTINATION ${TEST_INSTALL_DIRECTORY}/Release)
