#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source" )" >/dev/null 2>&1 && pwd)"
root="$(pwd)"
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

$directory/Beam/run_cmake.sh "$@"
for i in $applications; do
  if [ ! -d "Applications/$i" ]; then
    mkdir -p "Applications/$i"
  fi
  pushd "Applications/$i"
  $directory/Applications/$i/run_cmake.sh -DD="$root/Dependencies"
  popd
done
