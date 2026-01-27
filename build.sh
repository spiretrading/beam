#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""

main() {
  resolve_paths
  create_forwarding_scripts
  build_function "$@" "Beam"
  local targets=(
    "WebApi"
    "Applications/AdminClient"
    "Applications/ClientTemplate"
    "Applications/DataStoreProfiler"
    "Applications/HttpFileServer"
    "Applications/QueryStressTest"
    "Applications/QueueStressTest"
    "Applications/Scratch"
    "Applications/ServiceLocator"
    "Applications/ServiceProtocolProfiler"
    "Applications/ServletTemplate"
    "Applications/UidServer"
    "Applications/WebSocketEchoServer"
  )
  local jobs
  jobs=$(get_job_count)
  export -f build_function
  export DIRECTORY
  export ROOT
  parallel -j"$jobs" --no-notice build_function "$@" ::: "${targets[@]}"
}

resolve_paths() {
  local source="${BASH_SOURCE[0]}"
  while [[ -h "$source" ]]; do
    local dir="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
    source="$(readlink "$source")"
    [[ $source != /* ]] && source="$dir/$source"
  done
  DIRECTORY="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
  ROOT="$(pwd -P)"
}

create_forwarding_scripts() {
  if [[ ! -f "configure.sh" ]]; then
    ln -s "$DIRECTORY/configure.sh" configure.sh
  fi
  if [[ ! -f "build.sh" ]]; then
    ln -s "$DIRECTORY/build.sh" build.sh
  fi
}

build_function() {
  local location="${*: -1}"
  if [[ ! -d "$location" ]]; then
    mkdir -p "$location"
  fi
  pushd "$location" > /dev/null
  "$DIRECTORY/$location/build.sh" -DD="$ROOT/Beam/Dependencies" "${@:1:$#-1}"
  popd > /dev/null
}

get_job_count() {
  local cores mem jobs
  if [[ -f /proc/cpuinfo ]]; then
    cores=$(grep -c "processor" /proc/cpuinfo)
  else
    cores=$(sysctl -n hw.ncpu 2>/dev/null || echo 4)
  fi
  if [[ -f /proc/meminfo ]]; then
    mem=$(awk '/MemTotal/ {print int($2 / 4194304)}' /proc/meminfo)
  else
    mem=$(sysctl -n hw.memsize 2>/dev/null |
      awk '{print int($1 / 4294967296)}' || echo 4)
  fi
  ((cores -= 2))
  [[ $cores -lt 1 ]] && cores=1
  [[ $mem -lt 1 ]] && mem=1
  jobs=$((cores < mem ? cores : mem))
  echo "$jobs"
}

main "$@"
