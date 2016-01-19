#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/RegistryServer
printf "#define REGISTRY_SERVER_VERSION \""> ./../../Include/RegistryServer/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/RegistryServer/Version.hpp
printf \" >> ./../../Include/RegistryServer/Version.hpp
