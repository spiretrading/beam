#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/HttpFileServer
printf "#define HTTP_FILE_SERVER_VERSION \""> ./../../Include/HttpFileServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/HttpFileServer/Version.hpp
printf \" >> ./../../Include/HttpFileServer/Version.hpp
printf "\n" >> ./../../Include/HttpFileServer/Version.hpp
