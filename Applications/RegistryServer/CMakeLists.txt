cmake_minimum_required(VERSION 3.8)
project(RegistryServer)
execute_process(COMMAND "pwd" OUTPUT_VARIABLE PWD)
string(STRIP "${PWD}" PWD)
file(TO_CMAKE_PATH "${PWD}" PWD)
set(D "${PWD}/Dependencies" CACHE PATH "Path to dependencies directory.")
set(DEPENDENCIES_PATH "${D}")
file(TO_CMAKE_PATH "${DEPENDENCIES_PATH}" DEPENDENCIES_PATH)
include(../../Beam/Config/dependencies.cmake)
include_directories(${BEAM_INCLUDE_PATH})
include_directories(SYSTEM ${BOOST_INCLUDE_PATH})
include_directories(SYSTEM ${CRYPTOPP_INCLUDE_PATH})
include_directories(SYSTEM ${OPEN_SSL_INCLUDE_PATH})
include_directories(SYSTEM ${TCLAP_INCLUDE_PATH})
include_directories(SYSTEM ${YAML_INCLUDE_PATH})
link_directories(${BOOST_DEBUG_PATH})
link_directories(${BOOST_OPTIMIZED_PATH})
if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /WX /bigobj /std:c++17 /Wv:18")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /GL")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
  add_definitions(-DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)
  add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
  add_definitions(-D_HAS_AUTO_PTR_ETC=1)
  add_definitions(-DNOMINMAX)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS)
  add_definitions(-D_WIN32_WINNT=0x0501)
  add_definitions(-DWIN32_LEAN_AND_MEAN)
  add_definitions(/experimental:external)
  add_definitions(/external:W0)
  add_definitions(/external:anglebrackets)
endif()
if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX OR
    ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_RELEASE} -O2 -DNDEBUG")
endif()
if(CYGWIN)
  add_definitions(-D__USE_W32_SOCKETS)
endif()
if(${CMAKE_SYSTEM_NAME} STREQUAL "SunOS")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_RELEASE} -pthreads")
endif()
if(WIN32)
  execute_process(COMMAND cmd /c "CALL ${CMAKE_CURRENT_LIST_DIR}/version.bat")
elseif(UNIX)
  execute_process(COMMAND "${CMAKE_CURRENT_LIST_DIR}/version.sh")
endif()
include_directories(${PWD})
file(GLOB header_files ${PWD}/*.hpp)
file(GLOB source_files Source/*.cpp)
add_executable(RegistryServer ${header_files} ${source_files})
set_source_files_properties(${header_files} PROPERTIES HEADER_FILE_ONLY TRUE)
target_link_libraries(RegistryServer
  debug ${CRYPTOPP_LIBRARY_DEBUG_PATH}
  optimized ${CRYPTOPP_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_LIBRARY_OPTIMIZED_PATH}
  debug ${OPEN_SSL_BASE_LIBRARY_DEBUG_PATH}
  optimized ${OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH}
  debug ${YAML_LIBRARY_DEBUG_PATH}
  optimized ${YAML_LIBRARY_OPTIMIZED_PATH})
if(UNIX)
  target_link_libraries(RegistryServer
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
install(TARGETS RegistryServer DESTINATION ${PWD}/Application)