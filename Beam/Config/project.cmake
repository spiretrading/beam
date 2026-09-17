set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
set(D "${CMAKE_BINARY_DIR}/Dependencies" CACHE STRING
  "Path to dependencies folder.")
set(DEFAULT_BUILD_TYPE "Release")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE
    STRING "Choose the type of build." FORCE)
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
if(CMAKE_CONFIGURATION_TYPES AND
    NOT "MinSizeRel" IN_LIST CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES};MinSizeRel"
    CACHE STRING "Available build configurations." FORCE)
endif()

function(beam_configure_project source_directory binary_directory)
  cmake_parse_arguments(PARSE_ARGV 2 arguments "" "" "ENVIRONMENT")
  if(arguments_UNPARSED_ARGUMENTS OR arguments_KEYWORDS_MISSING_VALUES)
    message(FATAL_ERROR "Invalid beam_configure_project arguments.")
  endif()
  if(WIN32)
    set(configure_script
      cmd /c CALL "${source_directory}/configure.bat" -DD "${D}"
      "${CMAKE_BUILD_TYPE}")
  elseif(UNIX)
    set(configure_script "${source_directory}/configure.sh" "-DD=${D}"
      "${CMAKE_BUILD_TYPE}")
  endif()
  execute_process(COMMAND "${CMAKE_COMMAND}" -E env ${arguments_ENVIRONMENT}
    ${configure_script}
    WORKING_DIRECTORY "${binary_directory}" RESULT_VARIABLE configure_result
    OUTPUT_VARIABLE configure_output ERROR_VARIABLE configure_error
    OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_STRIP_TRAILING_WHITESPACE)
  if(NOT configure_result EQUAL 0)
    message(FATAL_ERROR "Configuration script failed with error:\n"
      "${configure_error}\nOutput:\n${configure_output}")
  endif()
endfunction()
