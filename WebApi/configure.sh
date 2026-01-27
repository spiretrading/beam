#!/bin/bash
set -o errexit
set -o pipefail
ROOT=""
DIRECTORY=""

main() {
  resolve_paths
  create_symlinks
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

create_symlinks() {
  if [[ ! -f "build.sh" ]]; then
    ln -s "$DIRECTORY/build.sh" build.sh
  fi
  if [[ ! -f "configure.sh" ]]; then
    ln -s "$DIRECTORY/configure.sh" configure.sh
  fi
  if [[ "$DIRECTORY" != "$ROOT" ]]; then
    if [[ ! -d "source" ]]; then
      ln -s "$DIRECTORY/source" source
    fi
    if [[ ! -f "package.json" ]]; then
      ln -s "$DIRECTORY/package.json" package.json
    fi
    if [[ ! -f "tsconfig.json" ]]; then
      ln -s "$DIRECTORY/tsconfig.json" tsconfig.json
    fi
  fi
}

main "$@"
