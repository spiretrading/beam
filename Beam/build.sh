#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""
SCRIPT_DIR=""
DEPENDENCIES=""
CONFIG=""

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
  cores=$((cores - 2))
  [[ $cores -lt 1 ]] && cores=1
  [[ $mem -lt 1 ]] && mem=1
  jobs=$((cores < mem ? cores : mem))
  echo "$jobs"
}

main() {
  resolve_paths
  parse_args "$@"
  shopt -s nocasematch
  if [[ "$CONFIG" == "clean" ]]; then
    shopt -u nocasematch
    clean_build "clean"
    return $?
  fi
  if [[ "$CONFIG" == "reset" ]]; then
    shopt -u nocasematch
    clean_build "reset"
    return $?
  fi
  shopt -u nocasematch
  configure || return 1
  run_build
  return $?
}

resolve_paths() {
  local source="${BASH_SOURCE[0]}"
  while [[ -h "$source" ]]; do
    local dir="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
    source="$(readlink "$source")"
    [[ $source != /* ]] && source="$dir/$source"
  done
  DIRECTORY="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
  SCRIPT_DIR="$DIRECTORY"
  ROOT="$(pwd -P)"
}

parse_args() {
  while [[ $# -gt 0 ]]; do
    case "$1" in
      -DD=*)
        DEPENDENCIES="${1#*=}"
        if [[ -z "$DEPENDENCIES" ]]; then
          echo "Error: -DD requires a path argument."
          return 1
        fi
        ;;
      -DD)
        if [[ -z "$2" || "$2" == -* ]]; then
          echo "Error: -DD requires a path argument."
          return 1
        fi
        DEPENDENCIES="$2"; shift
        ;;
      -D=*)
        DIRECTORY="${1#*=}"
        if [[ -z "$DIRECTORY" ]]; then
          echo "Error: -D requires a path argument."
          return 1
        fi
        ;;
      -D)
        if [[ -z "$2" || "$2" == -* ]]; then
          echo "Error: -D requires a path argument."
          return 1
        fi
        DIRECTORY="$2"; shift
        ;;
      *)     CONFIG="$1" ;;
    esac
    shift
  done
}

clean_build() {
  local clean_type="$1"
  local clean_error=0
  if [[ "$clean_type" == "reset" ]]; then
    if ! git rev-parse --show-toplevel > /dev/null 2>&1; then
      cmake -DBUILD_DIRECTORY:PATH="$ROOT" \
        -DSOURCE_DIRECTORY:PATH="$DIRECTORY" -P "$SCRIPT_DIR/Config/reset.cmake"
      return $?
    fi
    git clean -ffxd -- . || clean_error=1
  else
    if [[ ! -f "$ROOT/CMakeCache.txt" ]]; then
      return 0
    fi
    local scripts=("$ROOT"/CMakeFiles/beam_clean_*.cmake)
    if [[ ! -f "${scripts[0]}" ]]; then
      echo "Error: Run configure.sh before cleaning this build."
      return 1
    fi
    for script in "${scripts[@]}"; do
      cmake -P "$script" || clean_error=1
    done
  fi
  return "$clean_error"
}

configure() {
  if [[ -z "$CONFIG" ]]; then
    if [[ -f "CMakeFiles/config.txt" ]]; then
      CONFIG=$(< "CMakeFiles/config.txt")
    else
      CONFIG="Release"
    fi
  fi
  shopt -s nocasematch
  case "$CONFIG" in
    release)
      CONFIG="Release"
      ;;
    debug)
      CONFIG="Debug"
      ;;
    relwithdebinfo)
      CONFIG="RelWithDebInfo"
      ;;
    minsizerel)
      CONFIG="MinSizeRel"
      ;;
    *)
      shopt -u nocasematch
      echo "Error: Invalid configuration \"$CONFIG\"."
      return 1
      ;;
  esac
  shopt -u nocasematch
  if [[ -n "$DEPENDENCIES" ]]; then
    "$DIRECTORY/configure.sh" "$CONFIG" -DD="$DEPENDENCIES"
  else
    "$DIRECTORY/configure.sh" "$CONFIG"
  fi
}

run_build() {
  local jobs
  jobs=$(get_job_count)
  cmake --build "$ROOT" --config "$CONFIG" --parallel "$jobs" || return 1
  cmake --install "$ROOT" --config "$CONFIG" || return 1
  echo "$CONFIG" > "CMakeFiles/config.txt"
}

main "$@"
