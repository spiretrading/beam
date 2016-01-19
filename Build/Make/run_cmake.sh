#!/bin/bash
set -o errexit
set -o pipefail
directory=$(pwd)
if [ "$1" = "" ]
then
  build_type="Release"
else
  build_type=$1
fi
pushd $directory/../../Beam/Build/Make
./run_cmake.sh $build_type
popd

applications="AdminClient ClientTemplate QueryStressTest RegistryServer"
applications+=" ServiceLocator ServiceProtocolProfiler ServletTemplate"
applications+=" UidServer"

for i in $applications; do
  pushd $directory/../../Applications/$i/Build/Make
  ./run_cmake.sh $build_type
  popd
done
