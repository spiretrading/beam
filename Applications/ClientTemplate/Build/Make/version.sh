#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/ClientTemplate
printf "#define CLIENT_TEMPLATE_VERSION \""> ./../../Include/ClientTemplate/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/ClientTemplate/Version.hpp
printf \" >> ./../../Include/ClientTemplate/Version.hpp
printf "\n" >> ./../../Include/ClientTemplate/Version.hpp
