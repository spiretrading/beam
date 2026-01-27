#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""

main() {
  resolve_paths
  local version
  version=$(git --git-dir="$DIRECTORY/../../.git" \
    rev-list --count --first-parent HEAD)
  if [[ ! -f "Version.hpp" ]] || ! grep -q "$version" "Version.hpp"; then
    echo "#define ADMIN_CLIENT_VERSION \"$version\"" > "Version.hpp"
  fi
}

resolve_paths() {
  local source="${BASH_SOURCE[0]}"
  while [[ -h "$source" ]]; do
    local dir="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
    source="$(readlink "$source")"
    [[ $source != /* ]] && source="$dir/$source"
  done
  DIRECTORY="$(cd -P "$(dirname "$source")" >/dev/null && pwd -P)"
}

main "$@"
