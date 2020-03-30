#!/bin/bash
exit_status=0
let cores="`grep -c "processor" < /proc/cpuinfo`"
root="$(pwd)"
aspen_commit="28959f0b215738f62a005b9668de0d21971a6840"
build_aspen=0
if [ ! -d "aspen" ]; then
  git clone https://www.github.com/spiretrading/aspen
  if [ "$?" == "0" ]; then
    build_aspen=1
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
    ./configure.sh "-DD=$root" Debug
    ./build.sh
    ./configure.sh "-DD=$root" Release
    ./build.sh
  else
    pushd "$root"
    ./aspen/setup.sh
    popd
  fi
  popd
fi
if [ ! -d "cppunit-1.14.0" ]; then
  wget http://dev-www.libreoffice.org/src/cppunit-1.14.0.tar.gz --no-check-certificate
  if [ "$?" == "0" ]; then
    gzip -d -c cppunit-1.14.0.tar.gz | tar -x
    pushd cppunit-1.14.0
    touch configure.new
    cat configure | sed "s/\/\* automatically generated \*\//\$ac_prefix_conf_INP/" > configure.new
    mv configure.new configure
    chmod +x configure
    ./configure LDFLAGS='-ldl' --prefix="$root/cppunit-1.14.0"
    make -j $cores
    make install
    popd
  else
    exit_status=1
  fi
  rm -f cppunit-1.14.0.tar.gz
fi
if [ ! -d "cryptopp820" ]; then
  wget https://www.cryptopp.com/cryptopp820.zip -O cryptopp820.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    mkdir cryptopp820
    pushd cryptopp820
    unzip ../cryptopp820.zip
    make -j $cores
    make install PREFIX="$root/cryptopp820"
    popd
  else
    exit_status=1
  fi
  rm -f cryptopp820.zip
fi
if [ ! -d "doctest-2.3.6" ]; then
  wget https://github.com/onqtam/doctest/archive/2.3.6.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    unzip 2.3.6.zip
  else
    exit_status=1
  fi
  rm -f 2.3.6.zip
fi
if [ ! -d "mysql-connector-c-6.1.11-src" ]; then
  wget https://dev.mysql.com/get/Downloads/Connector-C/mysql-connector-c-6.1.11-src.tar.gz --no-check-certificate
  if [ "$?" == "0" ]; then
    gzip -d -c mysql-connector-c-6.1.11-src.tar.gz | tar -x
    pushd mysql-connector-c-6.1.11-src
    cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX:PATH="$root/mysql-connector-c-6.1.11-src"
    make -j $cores
    make install
    popd
  else
    exit_status=1
  fi
  rm -f mysql-connector-c-6.1.11-src.tar.gz
fi
if [ ! -d "openssl-1.1.1c" ]; then
  wget https://ftp.openssl.org/source/old/1.1.1/openssl-1.1.1c.tar.gz -O openssl-1.1.1c.tar.gz --no-check-certificate
  if [ "$?" == "0" ]; then
    gzip -d -c openssl-1.1.1c.tar.gz | tar -x
    pushd openssl-1.1.1c
    export LDFLAGS=-ldl
    ./config no-shared threads -fPIC -ldl --prefix="$root/openssl-1.1.1c"
    make -j $cores
    make test
    make install
    unset LDFLAGS
    popd
  else
    exit_status=1
  fi
  rm -f openssl-1.1.1c.tar.gz
fi
if [ ! -d "sqlite-amalgamation-3300100" ]; then
  wget https://www.sqlite.org/2019/sqlite-amalgamation-3300100.zip -O sqlite-amalgamation-3300100.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    unzip sqlite-amalgamation-3300100.zip
    pushd sqlite-amalgamation-3300100
    gcc -c -O2 -o sqlite3.lib -DSQLITE_USE_URI=1 -fPIC sqlite3.c
    popd
  else
    exit_status=1
  fi
  rm -f sqlite-amalgamation-3300100.zip
fi
if [ ! -d "tclap-1.2.2" ]; then
  wget https://github.com/mirror/tclap/archive/v1.2.2.zip -O v1.2.2.zip --no-check-certificate
  if [ "$?" == "0" ]; then
    unzip v1.2.2.zip
    pushd tclap-1.2.2
    ./configure
    make -j $cores
    popd
  else
    exit_status=1
  fi
  rm -f v1.2.2.zip
fi
viper_commit="009bbbb3e00269e9939a9342789334a2d56bf7f0"
if [ ! -d "viper" ]; then
  git clone https://www.github.com/spiretrading/viper
  if [ "$?" != "0" ]; then
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
if [ ! -d "boost_1_72_0" ]; then
  wget https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz -O boost_1_72_0.tar.gz --no-check-certificate
  if [ "$?" == "0" ]; then
    tar xvf boost_1_72_0.tar.gz
    pushd boost_1_72_0
    export BOOST_BUILD_PATH=$(pwd)
    ./bootstrap.sh
    ./b2 -j$cores --prefix="$root/boost_1_72_0" cxxflags="-std=c++17 -fPIC" install
    popd
    unset BOOST_BUILD_PATH
  else
    exit_status=1
  fi
  rm -f boost_1_72_0.tar.gz
fi
exit $exit_status
