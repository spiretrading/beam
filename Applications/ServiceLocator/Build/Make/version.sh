#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/ServiceLocator
printf "#define SERVICE_LOCATOR_VERSION \""> ./../../Include/ServiceLocator/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/ServiceLocator/Version.hpp
printf \" >> ./../../Include/ServiceLocator/Version.hpp
