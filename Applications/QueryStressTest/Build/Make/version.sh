#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/QueryStressTest
printf "#define QUERY_STRESS_TEST_VERSION \""> ./../../Include/QueryStressTest/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/QueryStressTest/Version.hpp
printf \" >> ./../../Include/QueryStressTest/Version.hpp
