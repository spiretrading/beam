#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/AdminClient
printf "#define ADMIN_CLIENT_VERSION \""> ./../../Include/AdminClient/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/AdminClient/Version.hpp
printf \" >> ./../../Include/AdminClient/Version.hpp
