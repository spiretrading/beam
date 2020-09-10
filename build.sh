#!/bin/bash
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
root=$(pwd -P)
if [ ! -f "configure.sh" ]; then
  ln -s "$directory/configure.sh" configure.sh
fi
if [ ! -f "build.sh" ]; then
  ln -s "$directory/build.sh" build.sh
fi
build_function() {
  location="${@: -1}"
  if [ ! -d "$location" ]; then
    mkdir -p "$location"
  fi
  pushd "$location"
  "$directory/$location/build.sh" -DD="$root/Beam/Dependencies" "${@:1:$#-1}"
  popd
}

export -f build_function
export directory
export root

build_function "$@" "Beam"
targets="WebApi"
targets+=" Applications/AdminClient"
targets+=" Applications/ClientTemplate"
targets+=" Applications/DataStoreProfiler"
targets+=" Applications/HttpFileServer"
targets+=" Applications/QueryStressTest"
targets+=" Applications/QueueStressTest"
targets+=" Applications/RegistryServer"
targets+=" Applications/Scratch"
targets+=" Applications/ServiceLocator"
targets+=" Applications/ServiceProtocolProfiler"
targets+=" Applications/ServletTemplate"
targets+=" Applications/UidServer"
targets+=" Applications/WebSocketEchoServer"

cores="`grep -c "processor" < /proc/cpuinfo` / 2 + 1"
mem="`grep -oP "MemTotal: +\K([[:digit:]]+)(?=.*)" < /proc/meminfo` / 4194304"
jobs="$(($cores<$mem?$cores:$mem))"
parallel -j$jobs --no-notice build_function "$@" ::: $targets
