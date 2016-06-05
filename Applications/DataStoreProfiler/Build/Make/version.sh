#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/DataStoreProfiler
printf "#define DATA_STORE_PROFILER_VERSION \""> ./../../Include/DataStoreProfiler/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/DataStoreProfiler/Version.hpp
printf \" >> ./../../Include/DataStoreProfiler/Version.hpp
printf "\n" >> ./../../Include/DataStoreProfiler/Version.hpp
