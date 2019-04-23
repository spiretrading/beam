#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd)"
root="$(pwd)"
if [ ! -f "configure.sh" ]; then
  printf "$directory/configure.sh \"\$@\"" > configure.sh
  chmod +x configure.sh
fi
if [ ! -f "build.sh" ]; then
  printf "$directory/build.sh \"\$@\"" > build.sh
  chmod +x build.sh
fi
targets="Beam"
targets+=" Applications/AdminClient"
targets+=" Applications/ClientTemplate"
targets+=" Applications/DataStoreProfiler"
targets+=" Applications/HttpFileServer"
targets+=" Applications/QueryStressTest"
targets+=" Applications/RegistryServer"
targets+=" Applications/ServiceLocator"
targets+=" Applications/ServiceProtocolProfiler"
targets+=" Applications/ServletTemplate"
targets+=" Applications/UidServer"
targets+=" Applications/WebSocketEchoServer"

for i in $targets; do
  if [ ! -d "$i" ]; then
    mkdir -p "$i"
  fi
  pushd "$i"
  if [ ! -f "configure.sh" ]; then
    printf "$directory/$i/configure.sh \"\$@\"" > configure.sh
    chmod +x configure.sh
  fi
  "$directory/$i/configure.sh" -DD="$root/Dependencies" "$@"
  popd
done
