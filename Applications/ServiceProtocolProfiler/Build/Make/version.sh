#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/ServiceProtocolProfiler
printf "#define SERVICE_PROTOCOL_PROFILER_VERSION \""> ./../../Include/ServiceProtocolProfiler/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/ServiceProtocolProfiler/Version.hpp
printf \" >> ./../../Include/ServiceProtocolProfiler/Version.hpp
