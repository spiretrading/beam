#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/ClientTemplate
printf "#define CLIENT_TEMPLATE_VERSION \""> ./../../Include/ClientTemplate/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/ClientTemplate/Version.hpp
printf \" >> ./../../Include/ClientTemplate/Version.hpp
