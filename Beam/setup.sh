#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""
CACHE_DIRECTORY=""
SETUP_HASH=""
DEPENDENCIES=()
REPOS=()

main() {
  resolve_paths
  CACHE_DIRECTORY="$ROOT/cache_files/beam"
  mkdir -p "$CACHE_DIRECTORY" || return 1
  SETUP_HASH=$(sha256 "$DIRECTORY/setup.sh") || return 1
  local cryptopp_url="https://github.com/weidai11/cryptopp/archive/refs/tags"
  add_dependency "cryptopp890" \
    "$cryptopp_url/CRYPTOPP_8_9_0.zip" \
    "b885403cb13d490bebe90f25fad7150b88857f7acf3bd8b9ca1cec04c9ec8a51" \
    "build_cryptopp"
  add_dependency "tclap-1.4.0-rc2" \
    "https://downloads.sourceforge.net/project/tclap/tclap-1.4.0-rc2.tar.bz2" \
    "ca52ce5badc477aeda59866601aad85c55e014c5400c15ed13e21fe7d0c1c5f7"
  add_dependency "yaml-cpp" \
    "https://github.com/jbeder/yaml-cpp/archive/refs/tags/yaml-cpp-0.9.0.zip" \
    "1c22709eb1fcde200c87ef4e878ce2c9477cc05eae84ebf1f72ec5b356468fee" \
    "build_yaml_cpp"
  add_dependency "zlib-1.3.1.2" \
    "https://github.com/madler/zlib/archive/refs/tags/v1.3.1.2.zip" \
    "2ae5dfd8a1df6cffff4b0cde7cde73f2986aefbaaddc22cc1a36537b0e948afc" \
    "build_zlib"
  add_dependency "boost_1_91_0" \
    "https://archives.boost.io/release/1.91.0/source/boost_1_91_0.zip" \
    "69c6f32fbda3c478fb310ec251e6699e5e584dbc71afb5425c2f6e98c9540a77" \
    "build_boost"
  add_repo "aspen" \
    "https://www.github.com/spiretrading/aspen" \
    "71e301a1b6329ecd5a0c6069121b2fe8c099929b" \
    "build_aspen"
  add_repo "viper" \
    "https://www.github.com/spiretrading/viper" \
    "6d3f11e68f3e6a1a97538420ce5044f5efd1578c" \
    "build_viper"
  install_dependencies || return 1
  install_repos || return 1
}

build_cryptopp() {
  local cores
  cores=$(get_core_count)
  make -j "$cores" || return 1
  make install PREFIX="$ROOT/cryptopp890" || return 1
}

build_yaml_cpp() {
  local cores
  cores=$(get_core_count)
  cmake --fresh -S . -B build -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DYAML_CPP_BUILD_TESTS=OFF -DYAML_CPP_BUILD_TOOLS=OFF || return 1
  cmake --build build --target yaml-cpp --parallel "$cores" || return 1
}

build_zlib() {
  local cores
  cores=$(get_core_count)
  export CFLAGS="-fPIC"
  cmake -DCMAKE_INSTALL_PREFIX:PATH="$ROOT/zlib-1.3.1.2" -G "Unix Makefiles" ||
    return 1
  make -j "$cores" || return 1
  make install || return 1
  unset CFLAGS
}

build_boost() {
  local cores
  cores=$(get_core_count)
  export BOOST_BUILD_PATH=$(pwd -P)
  ./bootstrap.sh || return 1
  ./b2 -j"$cores" --prefix="$ROOT/boost_1_91_0" \
    cxxflags="-std=c++23 -fPIC" install || return 1
  unset BOOST_BUILD_PATH
}

build_viper() {
  pushd "$ROOT" > /dev/null || return 1
  ./viper/setup.sh || { popd > /dev/null; return 1; }
  popd > /dev/null
}

build_aspen() {
  ./configure.sh -DD="$ROOT" || return 1
  ./build.sh Debug || return 1
  ./build.sh Release || return 1
}

sha256() {
  if command -v sha256sum >/dev/null; then
    sha256sum "$1" | cut -d" " -f1
  else
    shasum -a 256 "$1" | cut -d" " -f1
  fi
}

get_core_count() {
  nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4
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

add_dependency() {
  local name="$1"
  local url="$2"
  local hash="$3"
  local build="${4:-}"
  DEPENDENCIES+=("$name|$url|$hash|$build")
}

add_repo() {
  local name="$1"
  local url="$2"
  local commit="$3"
  local build="${4:-}"
  REPOS+=("$name|$url|$commit|$build")
}

install_dependencies() {
  for dep in "${DEPENDENCIES[@]}"; do
    IFS='|' read -r name url hash build <<< "$dep"
    download_and_extract "$name" "$url" "$hash" "$build" || return 1
  done
}

install_repos() {
  for repo in "${REPOS[@]}"; do
    IFS='|' read -r name url commit build <<< "$repo"
    clone_or_update_repo "$name" "$url" "$commit" "$build" || return 1
  done
}

download_and_extract() {
  local folder="$1"
  local build_marker="$CACHE_DIRECTORY/$folder.build_complete"
  local url="$2"
  local expected_hash="$3"
  local build_hash="$expected_hash $SETUP_HASH"
  local build_func="$4"
  local archive="${url##*/}"
  if [[ -d "$folder" && -f "$build_marker" ]] &&
      [[ "$(< "$build_marker")" == "$build_hash" ]]; then
    return 0
  fi
  rm -f "$build_marker" || return 1
  if [[ ! -f "$folder/.beam_extract_complete" ]] ||
      [[ "$(< "$folder/.beam_extract_complete")" != "$expected_hash" ]]; then
    rm -f "$folder/.beam_extract_complete" || return 1
    if [[ ! -f "$archive" ]]; then
      curl -fsSL -o "$archive" "$url" || {
        rm -f "$archive"
        return 1
      }
    fi
    local actual_hash
    actual_hash=$(sha256 "$archive") || return 1
    if [[ "$actual_hash" != "$expected_hash" ]]; then
      echo "Error: SHA256 mismatch for $archive."
      rm -f "$archive"
      return 1
    fi
    mkdir -p "$folder" || return 1
    if [[ "$archive" == *.zip ]]; then
      local archive_directory
      archive_directory=$(unzip -Z -1 "$archive" | sed -n '1s,/.*,,p') ||
        return 1
      if [[ -z "$archive_directory" || "$archive_directory" == "." ||
          "$archive_directory" == ".." ]]; then
        echo "Error: Invalid archive directory."
        return 1
      fi
      unzip -qo "$archive" -d "$folder" || return 1
      cp -R "$folder/$archive_directory/." "$folder/" || return 1
      rm -r "$folder/$archive_directory" || return 1
    else
      tar -xf "$archive" --strip-components=1 -C "$folder" || return 1
    fi
    echo "$expected_hash" > "$folder/.beam_extract_complete" || return 1
  fi
  if [[ -n "$build_func" ]]; then
    pushd "$folder" > /dev/null || return 1
    $build_func || { popd > /dev/null; return 1; }
    popd > /dev/null
  fi
  echo "$build_hash" > "$build_marker" || return 1
  if [[ -f "$archive" ]]; then
    rm -f "$archive" || return 1
  fi
}

clone_or_update_repo() {
  local repo_name="$1"
  local build_marker="$CACHE_DIRECTORY/$repo_name.build_complete"
  local repo_url="$2"
  local repo_commit="$3"
  local build_func="$4"
  local is_new_repo=0
  if [[ ! -d "$repo_name" ]]; then
    rm -f "$build_marker" || return 1
    git clone "$repo_url" "$repo_name" || return 1
    is_new_repo=1
  fi
  pushd "$repo_name" > /dev/null || return 1
  if [[ "$is_new_repo" -eq 1 ]]; then
    git checkout "$repo_commit" || { popd > /dev/null; return 1; }
  fi
  if ! git merge-base --is-ancestor "$repo_commit" HEAD; then
    git fetch origin || { popd > /dev/null; return 1; }
    rm -f "$build_marker" || { popd > /dev/null; return 1; }
    git checkout "$repo_commit" || { popd > /dev/null; return 1; }
  fi
  local repo_head
  repo_head=$(git rev-parse HEAD) || { popd > /dev/null; return 1; }
  local build_hash="$repo_head $SETUP_HASH"
  if [[ ! -f "$build_marker" ]] ||
      [[ "$(< "$build_marker")" != "$build_hash" ]]; then
    rm -f "$build_marker" || { popd > /dev/null; return 1; }
    if [[ -n "$build_func" ]]; then
      $build_func || { popd > /dev/null; return 1; }
    fi
    echo "$build_hash" > "$build_marker" ||
      { popd > /dev/null; return 1; }
  else
    (cd "$ROOT" && "./$repo_name/setup.sh") ||
      { popd > /dev/null; return 1; }
  fi
  popd > /dev/null
}

main "$@"
