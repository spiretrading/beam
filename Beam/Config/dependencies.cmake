if(WIN32)
  include("${CMAKE_CURRENT_LIST_DIR}/dependencies.windows.cmake")
else()
  include("${CMAKE_CURRENT_LIST_DIR}/dependencies.posix.cmake")
endif()
include("${PROJECT_BINARY_DIR}/Dependencies/aspen/Config/dependencies.cmake")
set(BEAM_INCLUDE_PATH "${CMAKE_CURRENT_LIST_DIR}/../Include")
set(BEAM_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../Source")
set(TCLAP_INCLUDE_PATH "${PROJECT_BINARY_DIR}/Dependencies/tclap-1.2.2/include")
set(VIPER_INCLUDE_PATH "${PROJECT_BINARY_DIR}/Dependencies/viper/Include")
