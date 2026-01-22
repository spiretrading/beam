#!/bin/bash
exit_status=0
source="${BASH_SOURCE[0]}"
while [ -h "$source" ]; do
  dir="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
  source="$(readlink "$source")"
  [[ $source != /* ]] && source="$dir/$source"
done
directory="$(cd -P "$(dirname "$source")" >/dev/null 2>&1 && pwd -P)"
root="$(pwd -P)"
if [ "$(uname -s)" = "Darwin" ]; then
  STAT='stat -x -t "%Y%m%d%H%M%S"'
else
  STAT='stat'
fi
if [ -f "cache_files/beam.txt" ]; then
  pt="$($STAT $directory/setup.sh | grep Modify | awk '{print $2 $3}')"
  mt="$($STAT cache_files/beam.txt | grep Modify | awk '{print $2 $3}')"
  if [[ ! "$pt" > "$mt" ]]; then
    exit 0
  fi
fi
cores="`grep -c "processor" < /proc/cpuinfo`"
aspen_commit="fa0573bf1df7cfb896c67bc09efbd1de0da94ba8"
build_aspen=0
if [ ! -d "aspen" ]; then
  git clone https://www.github.com/spiretrading/aspen
  if [ "$?" == "0" ]; then
    build_aspen=1
    pushd aspen
    git checkout "$aspen_commit"
    popd
  else
    rm -rf aspen
    exit_status=1
  fi
fi
if [ -d "aspen" ]; then
  pushd aspen
  if ! git merge-base --is-ancestor "$aspen_commit" HEAD; then
    git checkout master
    git pull
    git checkout "$aspen_commit"
    build_aspen=1
  fi
  if [ "$build_aspen" == "1" ]; then
    ./configure.sh "-DD=$root"
    ./build.sh Debug
    ./build.sh Release
  else
    pushd "$root"
    ./aspen/setup.sh
    popd
  fi
  popd
fi
if [ ! -d "cryptopp890" ]; then
  wget https://github.com/weidai11/cryptopp/archive/refs/tags/CRYPTOPP_8_9_0.zip -O cryptopp890.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    unzip cryptopp890.zip
    mv cryptopp-CRYPTOPP_8_9_0 cryptopp890
    pushd cryptopp890
    make -j $cores
    make install PREFIX="$root/cryptopp890"
    popd
  else
    exit_status=1
  fi
  rm -f cryptopp890.zip
fi
if [ ! -d "openssl-3.6.0" ]; then
  wget "https://github.com/openssl/openssl/releases/download/openssl-3.6.0/openssl-3.6.0.tar.gz" -O openssl-3.6.0.tar.gz --no-check-certificate
  if [ "$?" == "0" ]; then
    gzip -d -c openssl-3.6.0.tar.gz | tar -x
    pushd openssl-3.6.0
    export LDFLAGS=-ldl
    ./config no-shared threads -fPIC -ldl --prefix="$root/openssl-3.6.0"
    make -j $cores
    make test
    make install
    unset LDFLAGS
    popd
  else
    exit_status=1
  fi
  rm -f openssl-3.6.0.tar.gz
fi
if [ ! -d "mariadb-connector-c-3.4.8" ]; then
  wget https://github.com/mariadb-corporation/mariadb-connector-c/archive/refs/tags/v3.4.8.zip -O mariadb-connector-c-3.4.8.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    unzip mariadb-connector-c-3.4.8.zip
    pushd mariadb-connector-c-3.4.8
    export OPENSSL_ROOT_DIR="$root/openssl-3.6.0"
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./mariadb .
    make -j $cores
    make install
    unset OPENSSL_ROOT_DIR
    popd
  else
    exit_status=1
  fi
  rm -f mariadb-connector-c-3.4.8.zip
fi
if [ ! -d "sqlite-amalgamation-3510200" ]; then
  wget https://www.sqlite.org/2026/sqlite-amalgamation-3510200.zip -O sqlite-amalgamation-3510200.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    unzip sqlite-amalgamation-3510200.zip
    pushd sqlite-amalgamation-3510200
    gcc -c -O2 -o sqlite3.lib -DSQLITE_USE_URI=1 -fPIC sqlite3.c
    popd
  else
    exit_status=1
  fi
  rm -f sqlite-amalgamation-3510200.zip
fi
if [ ! -d "tclap-1.2.5" ]; then
  wget https://github.com/mirror/tclap/archive/v1.2.5.zip -O v1.2.5.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    unzip v1.2.5.zip
    pushd tclap-1.2.5
    ./autotools.sh
    ./configure
    make -j $cores
    popd
  else
    exit_status=1
  fi
  rm -f v1.2.5.zip
fi
viper_commit="c5adf4c8101d30077fc0fa35cfb1b7b96bf2d1fe"
if [ ! -d "viper" ]; then
  git clone https://www.github.com/spiretrading/viper
  if [ "$?" == "0" ]; then
    pushd viper
    git checkout "$viper_commit"
    popd
  else
    rm -rf viper
    exit_status=1
  fi
fi
if [ -d "viper" ]; then
  pushd viper
  if ! git merge-base --is-ancestor "$viper_commit" HEAD; then
    git checkout master
    git pull
    git checkout "$viper_commit"
  fi
  popd
fi
if [ ! -d "yaml-cpp-0.6.2" ]; then
  git clone --branch yaml-cpp-0.6.2 https://github.com/jbeder/yaml-cpp.git yaml-cpp-0.6.2
  if [ "$?" == "0" ]; then
    pushd yaml-cpp-0.6.2
    mkdir build
    popd
    pushd yaml-cpp-0.6.2/build
    export CFLAGS="-fPIC"
    export CXXFLAGS="-fPIC"
    cmake -DCMAKE_INSTALL_PREFIX:PATH="$root/yaml-cpp" ..
    make -j $cores
    unset CFLAGS
    unset CXXFLAGS
    popd
  else
    rm -rf yaml-cpp-0.6.2
    exit_status=1
  fi
fi
if [ ! -d "zlib-1.3.1.2" ]; then
  wget "https://github.com/madler/zlib/archive/refs/tags/v1.3.1.2.zip" --no-check-certificate
  if [ "$?" == "0" ]; then
    unzip v1.3.1.2.zip
    pushd zlib-1.3.1.2
    export CFLAGS="-fPIC"
    cmake -DCMAKE_INSTALL_PREFIX:PATH="$root/zlib-1.3.1.2" -G "Unix Makefiles"
    make -j $cores
    make install
    unset CFLAGS
    popd
  else
    exit_status=1
  fi
  rm -f v1.3.1.2.zip
fi
if [ ! -d "boost_1_90_0" ]; then
  wget "https://archives.boost.io/release/1.90.0/source/boost_1_90_0.zip" -O boost_1_90_0.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    unzip boost_1_90_0.zip
    pushd boost_1_90_0
    export BOOST_BUILD_PATH=$(pwd -P)
    ./bootstrap.sh
    ./b2 -j$cores --prefix="$root/boost_1_90_0" cxxflags="-std=c++20 -fPIC" install
    popd
    unset BOOST_BUILD_PATH
  else
    exit_status=1
  fi
  rm -f boost_1_90_0.zip
fi
if [ ! -d cache_files ]; then
  mkdir cache_files
fi
echo timestamp > cache_files/beam.txt
exit $exit_status
