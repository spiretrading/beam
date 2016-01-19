#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/ServletTemplate
printf "#define SERVLET_TEMPLATE_VERSION \""> ./../../Include/ServletTemplate/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/ServletTemplate/Version.hpp
printf \" >> ./../../Include/ServletTemplate/Version.hpp
