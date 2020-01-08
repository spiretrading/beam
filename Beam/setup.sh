#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"
root="$(pwd)"
aspen_commit="986385c8fa17270165496c3bcd8c454da0ad9580"
build_aspen=0
if [ ! -d "aspen" ]; then
  git clone https://www.github.com/eidolonsystems/aspen
  build_aspen=1
fi
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
if [ ! -d "cppunit-1.14.0" ]; then
  wget http://dev-www.libreoffice.org/src/cppunit-1.14.0.tar.gz --no-check-certificate
  if [ -f cppunit-1.14.0.tar.gz ]; then
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
    rm cppunit-1.14.0.tar.gz
  fi
fi
if [ ! -d "cryptopp820" ]; then
  wget https://www.cryptopp.com/cryptopp820.zip -O cryptopp820.zip --no-check-certificate
  if [ -f cryptopp820.zip ]; then
    mkdir cryptopp820
    pushd cryptopp820
    unzip ../cryptopp820.zip
    make -j $cores
    make install PREFIX="$root/cryptopp820"
    popd
    rm cryptopp820.zip
  fi
fi
if [ ! -d "doctest-2.3.6" ]; then
  wget https://github.com/onqtam/doctest/archive/2.3.6.zip --no-check-certificate
  if [ -f "2.3.6.zip" ]; then
    unzip 2.3.6.zip
    rm 2.3.6.zip
  fi
fi
if [ ! -d "mysql-connector-c-6.1.11-src" ]; then
  wget https://dev.mysql.com/get/Downloads/Connector-C/mysql-connector-c-6.1.11-src.tar.gz --no-check-certificate
  if [ -f mysql-connector-c-6.1.11-src.tar.gz ]; then
    gzip -d -c mysql-connector-c-6.1.11-src.tar.gz | tar -x
    pushd mysql-connector-c-6.1.11-src
    cmake -G "Unix Makefiles" -DCMAKE_INSTALL_PREFIX:PATH="$root/mysql-connector-c-6.1.11-src"
    make -j $cores
    make install
    popd
    rm mysql-connector-c-6.1.11-src.tar.gz
  fi
fi
if [ ! -d "openssl-1.1.1c" ]; then
  wget https://ftp.openssl.org/source/old/1.1.1/openssl-1.1.1c.tar.gz -O openssl-1.1.1c.tar.gz --no-check-certificate
  if [ -f openssl-1.1.1c.tar.gz ]; then
    gzip -d -c openssl-1.1.1c.tar.gz | tar -x
    pushd openssl-1.1.1c
    export LDFLAGS=-ldl
    ./config no-shared threads -fPIC -ldl --prefix="$root/openssl-1.1.1c"
    make -j $cores
    make test
    make install
    unset LDFLAGS
    popd
    rm openssl-1.1.1c.tar.gz
  fi
fi
if [ ! -d "Python-3.6.7" ]; then
  wget https://www.python.org/ftp/python/3.6.7/Python-3.6.7.tgz --no-check-certificate
  if [ -f Python-3.6.7.tgz ]; then
    gzip -d -c Python-3.6.7.tgz | tar -xf -
    pushd Python-3.6.7
    export CFLAGS="-fPIC"
    ./configure --prefix="$root/Python-3.6.7"
    make -j $cores
    make install
    unset CFLAGS
    popd
    rm Python-3.6.7.tgz
  fi
fi
if [ ! -d "sqlite-amalgamation-3300100" ]; then
  wget https://www.sqlite.org/2019/sqlite-amalgamation-3300100.zip -O sqlite-amalgamation-3300100.zip --no-check-certificate
  if [ -f sqlite-amalgamation-3300100.zip ]; then
    unzip sqlite-amalgamation-3300100.zip
    pushd sqlite-amalgamation-3300100
    gcc -c -O2 -o sqlite3.lib -DSQLITE_USE_URI=1 -fPIC sqlite3.c
    popd
    rm sqlite-amalgamation-3300100.zip
  fi
fi
if [ ! -d "tclap-1.2.2" ]; then
  wget https://github.com/mirror/tclap/archive/v1.2.2.zip -O v1.2.2.zip --no-check-certificate
  if [ -f v1.2.2.zip ]; then
    unzip v1.2.2.zip
    pushd tclap-1.2.2
    ./configure
    make -j $cores
    popd
    rm v1.2.2.zip
  fi
fi
viper_commit="3998912cecaaa041b2dea37485905b3345797744"
if [ ! -d "viper" ]; then
  git clone https://www.github.com/eidolonsystems/viper
fi
pushd viper
if ! git merge-base --is-ancestor "$viper_commit" HEAD; then
  git checkout master
  git pull
  git checkout "$viper_commit"
fi
popd
if [ ! -d "yaml-cpp-0.6.2" ]; then
  git clone --branch yaml-cpp-0.6.2 https://github.com/jbeder/yaml-cpp.git yaml-cpp-0.6.2
  if [ -d "yaml-cpp-0.6.2" ]; then
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
  fi
fi
if [ ! -d "zlib-1.2.11" ]; then
  wget https://github.com/madler/zlib/archive/v1.2.11.zip --no-check-certificate
  if [ -f v1.2.11.zip ]; then
    unzip v1.2.11.zip
    pushd zlib-1.2.11
    export CFLAGS="-fPIC"
    cmake -DCMAKE_INSTALL_PREFIX:PATH="$root/zlib-1.2.11" -G "Unix Makefiles"
    make -j $cores
    make install
    unset CFLAGS
    popd
    rm v1.2.11.zip
  fi
fi
if [ ! -d "boost_1_72_0" ]; then
  wget https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.tar.gz -O boost_1_72_0.tar.gz --no-check-certificate
  if [ -f boost_1_72_0.tar.gz ]; then
    tar xvf boost_1_72_0.tar.gz
    pushd boost_1_72_0
    export BOOST_BUILD_PATH=$(pwd)
    ./bootstrap.sh
    ./b2 -j$cores --prefix="$root/boost_1_72_0" cxxflags="-std=c++17 -fPIC" install
    popd
    unset BOOST_BUILD_PATH
    rm boost_1_72_0.tar.gz
  fi
fi
