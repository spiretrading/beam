#!/bin/bash
set -o errexit
set -o pipefail
DIRECTORY=""
ROOT=""
CACHE_NAME=""
SETUP_HASH=""
DEPENDENCIES=()
REPOS=()

main() {
  resolve_paths
  check_cache "beam" || exit 0
  add_dependency "cryptopp890" \
    "https://github.com/weidai11/cryptopp/archive/refs/tags/CRYPTOPP_8_9_0.zip" \
    "b885403cb13d490bebe90f25fad7150b88857f7acf3bd8b9ca1cec04c9ec8a51" \
    "build_cryptopp"
  add_dependency "openssl-3.6.0" \
    "https://github.com/openssl/openssl/releases/download/openssl-3.6.0/openssl-3.6.0.tar.gz" \
    "b6a5f44b7eb69e3fa35dbf15524405b44837a481d43d81daddde3ff21fcbb8e9" \
    "build_openssl"
  add_dependency "mariadb-connector-c-3.4.9" \
    "https://github.com/mariadb-corporation/mariadb-connector-c/archive/refs/tags/v3.4.9.zip" \
    "2342f6e58907f7431b5ccafb8b8e744b6b0e64174d72395d2330576b8a535fb6" \
    "build_mariadb"
  add_dependency "sqlite-amalgamation-3510200" \
    "https://www.sqlite.org/2026/sqlite-amalgamation-3510200.zip" \
    "6e2a845a493026bdbad0618b2b5a0cf48584faab47384480ed9f592d912f23ec" \
    "build_sqlite"
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
    "ca15c76178a9f883758ef7d6e13ce4839e8d7930" \
    "build_aspen"
  add_repo "viper" \
    "https://www.github.com/spiretrading/viper" \
    "76d3876dff8307602a827a7e2edcc2bed7885390"
  install_dependencies || return 1
  install_repos || return 1
  commit
}

build_cryptopp() {
  local cores
  cores=$(get_core_count)
  make -j "$cores" || return 1
  make install PREFIX="$ROOT/cryptopp890" || return 1
}

build_openssl() {
  local cores
  cores=$(get_core_count)
  popd > /dev/null
  mv "openssl-3.6.0" "openssl-3.6.0-build"
  pushd "openssl-3.6.0-build" > /dev/null
  export LDFLAGS=-ldl
  ./config no-shared threads -fPIC -ldl --prefix="$ROOT/openssl-3.6.0" ||
    return 1
  make -j "$cores" || return 1
  make test || return 1
  make install || return 1
  unset LDFLAGS
  popd > /dev/null
  rm -rf "openssl-3.6.0-build"
  pushd "openssl-3.6.0" > /dev/null
}

build_mariadb() {
  local cores
  cores=$(get_core_count)
  export OPENSSL_ROOT_DIR="$ROOT/openssl-3.6.0"
  cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=./mariadb \
    -DCLIENT_PLUGIN_CACHING_SHA2_PASSWORD=STATIC . || return 1
  make -j "$cores" || return 1
  make install || return 1
  unset OPENSSL_ROOT_DIR
}

build_sqlite() {
  gcc -c -O2 -o sqlite3.lib -DSQLITE_USE_URI=1 -fPIC sqlite3.c || return 1
}

build_yaml_cpp() {
  local cores
  cores=$(get_core_count)
  mkdir -p build || return 1
  pushd build > /dev/null
  cmake -DCMAKE_POSITION_INDEPENDENT_CODE=ON .. ||
    { popd > /dev/null; return 1; }
  cmake --build . --config Debug -j "$cores" || { popd > /dev/null; return 1; }
  cmake --build . --config Release -j "$cores" ||
    { popd > /dev/null; return 1; }
  popd > /dev/null
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
    cxxflags="-std=c++20 -fPIC" install || return 1
  unset BOOST_BUILD_PATH
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

check_cache() {
  CACHE_NAME="$1"
  SETUP_HASH=$(sha256 "$DIRECTORY/setup.sh")
  if [[ -f "cache_files/$CACHE_NAME.txt" ]]; then
    local cached_hash
    cached_hash=$(cat "cache_files/$CACHE_NAME.txt")
    if [[ "$SETUP_HASH" == "$cached_hash" ]]; then
      return 1
    fi
  fi
  return 0
}

commit() {
  if [[ ! -d "cache_files" ]]; then
    mkdir -p cache_files || return 1
  fi
  echo "$SETUP_HASH" > "cache_files/$CACHE_NAME.txt"
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
  local url="$2"
  local expected_hash="$3"
  local build_func="$4"
  local archive="${url##*/}"
  if [[ -d "$folder" ]]; then
    return 0
  fi
  if [[ ! -f "$archive" ]]; then
    curl -fsSL -o "$archive" "$url" || return 1
  fi
  local actual_hash
  actual_hash=$(sha256 "$archive")
  if [[ "$actual_hash" != "$expected_hash" ]]; then
    echo "Error: SHA256 mismatch for $archive."
    echo "  Expected: $expected_hash"
    echo "  Actual:   $actual_hash"
    rm -f "$archive"
    return 1
  fi
  mkdir -p "$folder" || return 1
  if [[ "$archive" == *.zip ]]; then
    unzip -q "$archive" -d "$folder" || { rm -rf "$folder"; return 1; }
  else
    tar -xf "$archive" -C "$folder" || { rm -rf "$folder"; return 1; }
  fi
  flatten_directory "$folder"
  if [[ -n "$build_func" ]]; then
    pushd "$folder" > /dev/null
    $build_func || { popd > /dev/null; return 1; }
    popd > /dev/null
  fi
  rm -f "$archive"
}

flatten_directory() {
  local folder="$1"
  local dir_count=0
  local file_count=0
  local single_dir=""
  for d in "$folder"/*/; do
    if [[ -d "$d" ]]; then
      ((dir_count += 1))
      single_dir="$d"
    fi
  done
  for f in "$folder"/*; do
    if [[ -f "$f" ]]; then
      ((file_count += 1))
    fi
  done
  if [[ "$dir_count" -eq 1 ]] && [[ "$file_count" -eq 0 ]]; then
    shopt -s dotglob
    mv "$single_dir"* "$folder/" 2>/dev/null || true
    shopt -u dotglob
    rmdir "$single_dir" 2>/dev/null || true
  fi
}

clone_or_update_repo() {
  local repo_name="$1"
  local repo_url="$2"
  local repo_commit="$3"
  local build_func="$4"
  local needs_build=0
  if [[ ! -d "$repo_name" ]]; then
    git clone "$repo_url" "$repo_name" || { rm -rf "$repo_name"; return 1; }
    pushd "$repo_name" > /dev/null
    git checkout "$repo_commit"
    popd > /dev/null
    needs_build=1
  else
    pushd "$repo_name" > /dev/null
    if ! git merge-base --is-ancestor "$repo_commit" HEAD; then
      git checkout master
      git pull
      git checkout "$repo_commit"
      needs_build=1
    fi
    popd > /dev/null
  fi
  if [[ "$needs_build" == "1" ]] && [[ -n "$build_func" ]]; then
    pushd "$repo_name" > /dev/null
    $build_func || { popd > /dev/null; return 1; }
    popd > /dev/null
  fi
}

main "$@"
