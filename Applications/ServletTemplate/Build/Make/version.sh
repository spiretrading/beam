#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/ServletTemplate
printf "#define SERVLET_TEMPLATE_VERSION \""> ./../../Include/ServletTemplate/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/ServletTemplate/Version.hpp
printf \" >> ./../../Include/ServletTemplate/Version.hpp
printf "\n" >> ./../../Include/ServletTemplate/Version.hpp
