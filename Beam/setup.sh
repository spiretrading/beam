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
aspen_commit="c6e27744a15496a5847c58536e5ada190511433e"
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
if [ ! -d "cryptopp840" ]; then
  wget https://www.cryptopp.com/cryptopp840.zip -O cryptopp840.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    mkdir cryptopp840
    pushd cryptopp840
    unzip ../cryptopp840.zip
    make -j $cores
    make install PREFIX="$root/cryptopp840"
    popd
  else
    exit_status=1
  fi
  rm -f cryptopp840.zip
fi
if [ ! -d "mariadb-connector-c-3.1.13" ]; then
  wget https://github.com/MariaDB/mariadb-connector-c/archive/v3.1.13.zip -O mariadb-connector-c-3.1.13.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    unzip mariadb-connector-c-3.1.13.zip
    pushd mariadb-connector-c-3.1.13
    cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./mariadb .
    make -j $cores
    make install
    popd
  else
    exit_status=1
  fi
  rm -f mariadb-connector-c-3.1.13.zip
fi
if [ ! -d "openssl-1.1.1h" ]; then
  wget https://ftp.openssl.org/source/old/1.1.1/openssl-1.1.1h.tar.gz -O openssl-1.1.1h.tar.gz --no-check-certificate
  if [ "$?" == "0" ]; then
    gzip -d -c openssl-1.1.1h.tar.gz | tar -x
    pushd openssl-1.1.1h
    export LDFLAGS=-ldl
    ./config no-shared threads -fPIC -ldl --prefix="$root/openssl-1.1.1h"
    make -j $cores
    make test
    make install
    unset LDFLAGS
    popd
  else
    exit_status=1
  fi
  rm -f openssl-1.1.1h.tar.gz
fi
if [ ! -d "sqlite-amalgamation-3340000" ]; then
  wget https://www.sqlite.org/2020/sqlite-amalgamation-3340000.zip -O sqlite-amalgamation-3340000.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    unzip sqlite-amalgamation-3340000.zip
    pushd sqlite-amalgamation-3340000
    gcc -c -O2 -o sqlite3.lib -DSQLITE_USE_URI=1 -fPIC sqlite3.c
    popd
  else
    exit_status=1
  fi
  rm -f sqlite-amalgamation-3340000.zip
fi
if [ ! -d "tclap-1.2.2" ]; then
  wget https://github.com/mirror/tclap/archive/v1.2.2.zip -O v1.2.2.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    unzip v1.2.2.zip
    pushd tclap-1.2.2
    ./autotools.sh
    ./configure
    make -j $cores
    popd
  else
    exit_status=1
  fi
  rm -f v1.2.2.zip
fi
viper_commit="1b68c4bad88fae9ce5cde57a3079ad167af8fed6"
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
if [ ! -d "zlib-1.2.11" ]; then
  wget https://github.com/madler/zlib/archive/v1.2.11.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    unzip v1.2.11.zip
    pushd zlib-1.2.11
    export CFLAGS="-fPIC"
    cmake -DCMAKE_INSTALL_PREFIX:PATH="$root/zlib-1.2.11" -G "Unix Makefiles"
    make -j $cores
    make install
    unset CFLAGS
    popd
  else
    exit_status=1
  fi
  rm -f v1.2.11.zip
fi
if [ ! -d "boost_1_77_0" ]; then
  wget https://boostorg.jfrog.io/artifactory/main/release/1.77.0/source/boost_1_77_0.tar.gz -O boost_1_77_0.tar.gz --no-check-certificate
  if [ "$?" == "0" ]; then
    tar xvf boost_1_77_0.tar.gz
    pushd boost_1_77_0
    export BOOST_BUILD_PATH=$(pwd -P)
    ./bootstrap.sh
    ./b2 -j$cores --prefix="$root/boost_1_77_0" cxxflags="-std=c++20 -fPIC" install
    popd
    unset BOOST_BUILD_PATH
  else
    exit_status=1
  fi
  rm -f boost_1_77_0.tar.gz
fi
if [ ! -d cache_files ]; then
  mkdir cache_files
fi
echo timestamp > cache_files/beam.txt
exit $exit_status
