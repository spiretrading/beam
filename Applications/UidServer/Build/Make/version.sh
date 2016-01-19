#!/bin/bash
set -o errexit
set -o pipefail
mkdir -p ./../../Include/UidServer
printf "#define UID_SERVER_VERSION \""> ./../../Include/UidServer/Version.hpp
hg id -n | tr -d "\n" >> ./../../Include/UidServer/Version.hpp
printf \" >> ./../../Include/UidServer/Version.hpp
