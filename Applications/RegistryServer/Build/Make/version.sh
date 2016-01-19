#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/RegistryServer
printf "#define REGISTRY_SERVER_VERSION \""> ./../../Include/RegistryServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/RegistryServer/Version.hpp
printf \" >> ./../../Include/RegistryServer/Version.hpp
printf "\n" >> ./../../Include/RegistryServer/Version.hpp
