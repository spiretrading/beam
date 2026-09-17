#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""
DEPENDENCIES=""
ARGS=()

main() {
  resolve_paths
  parse_args "$@" || return 1
  create_forwarding_scripts
  local targets=(
    "Beam"
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
  for target in "${targets[@]}"; do
    configure_target "$target" "${ARGS[@]}"
  done
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

parse_args() {
  DEPENDENCIES="$ROOT/Beam/Dependencies"
  ARGS=()
  while [[ $# -gt 0 ]]; do
    local arg="$1"
    if [[ "$arg" == "-DD" ]]; then
      shift
      if [[ $# -eq 0 || -z "$1" ]]; then
        echo "Error: -DD requires a path argument."
        return 1
      fi
      DEPENDENCIES="$1"
    elif [[ "$arg" == -DD=* ]]; then
      DEPENDENCIES="${arg#-DD=}"
      if [[ -z "$DEPENDENCIES" ]]; then
        echo "Error: -DD requires a path argument."
        return 1
      fi
    else
      ARGS+=("$arg")
    fi
    shift
  done
  if [[ "$DEPENDENCIES" != /* ]]; then
    DEPENDENCIES="$ROOT/$DEPENDENCIES"
  fi
}

create_forwarding_scripts() {
  for script in configure build; do
    if [[ ! -f "$script.sh" ]]; then
      printf '#!/bin/bash\nexec %q "$@"\n' "$DIRECTORY/$script.sh" \
        > "$script.sh" || return 1
      chmod +x "$script.sh" || return 1
    fi
  done
  mkdir -p Applications || return 1
  if [[ ! -f Applications/install_python.sh ]]; then
    printf '#!/bin/bash\nexec %q "$@"\n' \
      "$DIRECTORY/Applications/install_python.sh" \
      > Applications/install_python.sh || return 1
    chmod +x Applications/install_python.sh || return 1
  fi
  for script in setup stress_test; do
    if [[ ! -f "Applications/$script.py" ]]; then
      ln -s "$DIRECTORY/Applications/$script.py" "Applications/$script.py" ||
        return 1
    fi
  done
}

configure_target() {
  local target="$1"
  shift
  if [[ ! -d "$target" ]]; then
    mkdir -p "$target"
  fi
  pushd "$target" > /dev/null
  "$DIRECTORY/$target/configure.sh" -DD="$DEPENDENCIES" "$@"
  popd > /dev/null
}

main "$@"
