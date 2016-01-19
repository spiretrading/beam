#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/QueryStressTest
printf "#define QUERY_STRESS_TEST_VERSION \""> ./../../Include/QueryStressTest/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/QueryStressTest/Version.hpp
printf \" >> ./../../Include/QueryStressTest/Version.hpp
printf "\n" >> ./../../Include/QueryStressTest/Version.hpp
