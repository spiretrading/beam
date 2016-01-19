#!/bin/bash
set -o errexit
set -o pipefail
directory=$(pwd)
if [ "$1" = "" ]
then
  config="install"
else
  config=$1
fi

build_function() {
  pushd $directory/../../Applications/$1/Build/Make
  ./build.sh $config
  popd
}

let cores="`grep -c "processor" < /proc/cpuinfo` / 2 + 1"

pushd $directory/../../Beam/Build/Make
./build.sh $config
popd
export -f build_function
export directory
export config
applications="AdminClient ClientTemplate QueryStressTest RegistryServer"
applications+=" ServiceLocator ServiceProtocolProfiler ServletTemplate"
applications+=" UidServer"
parallel -j$cores --no-notice build_function ::: $applications
