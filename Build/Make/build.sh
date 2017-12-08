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
let mem="`grep -oP "MemTotal: +\K([[:digit:]]+)(?=.*)" < /proc/meminfo` / 4194304"
let jobs="$(($cores<$mem?$cores:$mem))"

pushd $directory/../../Beam/Build/Make
./build.sh $config
popd
export -f build_function
export directory
export config
applications="AdminClient"
applications+=" ClientTemplate"
applications+=" DataStoreProfiler"
applications+=" HttpFileServer"
applications+=" QueryStressTest"
applications+=" RegistryServer"
applications+=" ServiceLocator"
applications+=" ServiceProtocolProfiler"
applications+=" ServletTemplate"
applications+=" UidServer"
applications+=" WebSocketEchoServer"
parallel -j$jobs --no-notice build_function ::: $applications

pushd $directory/../../Documents/sphinx
make clean
make html
popd
