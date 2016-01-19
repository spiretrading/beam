#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/UidServer
printf "#define UID_SERVER_VERSION \""> ./../../Include/UidServer/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n" >> ./../../Include/UidServer/Version.hpp
printf \" >> ./../../Include/UidServer/Version.hpp
printf "\n" >> ./../../Include/UidServer/Version.hpp
