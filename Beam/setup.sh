#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"
root="$(pwd)"
aspen_commit="a07ecd17fc4d8e979f35ca6a927193a7662151ad"
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
  ./configure.sh "-DD=$root"
  ./build.sh
else
  pushd "$root"
  ./aspen/aspen/setup.sh
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
    rm -f cppunit-1.14.0.tar.gz
  fi
fi
if [ ! -d "cryptopp565" ]; then
  wget https://github.com/weidai11/cryptopp/archive/b0f3b8ce1761e7ab9a3ead46fb7403fb38dd3723.zip -O cryptopp565.zip --no-check-certificate
  if [ -f cryptopp565.zip ]; then
    unzip cryptopp565.zip
    mv cryptopp-b0f3b8ce1761e7ab9a3ead46fb7403fb38dd3723 cryptopp565
    pushd cryptopp565
    make -j $cores
    make install PREFIX="$root/cryptopp565"
    popd
    rm -f cryptopp565.zip
  fi
fi
if [ ! -d "doctest-2.3.4" ]; then
  wget https://github.com/onqtam/doctest/archive/2.3.4.zip --no-check-certificate
  if [ -f "2.3.4.zip" ]; then
    unzip 2.3.4.zip
    rm -f 2.3.4.zip
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
    rm -f mysql-connector-c-6.1.11-src.tar.gz
  fi
fi
if [ ! -d "openssl-1.0.2g" ]; then
  wget https://www.openssl.org/source/openssl-1.0.2g.tar.gz --no-check-certificate
  if [ -f openssl-1.0.2g.tar.gz ]; then
    gzip -d -c openssl-1.0.2g.tar.gz | tar -x
    pushd openssl-1.0.2g
    export LDFLAGS=-ldl
    ./config no-shared threads -fPIC -ldl --prefix="$root/openssl-1.0.2g"
    make -j $cores
    make test
    make install
    unset LDFLAGS
    popd
    rm openssl-1.0.2g.tar.gz
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
if [ ! -d "sqlite-amalgamation-3230100" ]; then
  wget https://www.sqlite.org/2018/sqlite-amalgamation-3230100.zip --no-check-certificate
  if [ -f sqlite-amalgamation-3230100.zip ]; then
    unzip sqlite-amalgamation-3230100.zip
    pushd sqlite-amalgamation-3230100
    gcc -c -O2 -o sqlite3.lib -DSQLITE_USE_URI=1 -fPIC sqlite3.c
    popd
    rm sqlite-amalgamation-3230100.zip
  fi
fi
if [ ! -d "tclap-1.2.1" ]; then
  wget "https://downloads.sourceforge.net/project/tclap/tclap-1.2.1.tar.gz?r=&ts=1309913922&use_mirror=superb-sea2" -O tclap-1.2.1.tar.gz --no-check-certificate
  if [ -f tclap-1.2.1.tar.gz ]; then
    gzip -d -c tclap-1.2.1.tar.gz | tar -x
    pushd tclap-1.2.1
    ./configure
    make -j $cores
    popd
    rm -f tclap-1.2.1.tar.gz
  fi
fi
viper_commit="84a99535652ed914ec5c6c9bf55477089100ae4a"
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
    rm -f v1.2.11.zip
  fi
fi
if [ ! -d "boost_1_70_0" ]; then
  wget https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.tar.gz -O boost_1_70_0.tar.gz --no-check-certificate
  if [ -f boost_1_70_0.tar.gz ]; then
    tar xvf boost_1_70_0.tar.gz
    pushd boost_1_70_0
    pushd tools/build/src
    printf "using python : 3.6 : $root/Python-3.6.7 : $root/Python-3.6.7/include/python3.6m : $root/Python-3.6.7 ;\n" > user-config.jam
    popd
    export BOOST_BUILD_PATH=$(pwd)
    ./bootstrap.sh
    ./b2 -j$cores --prefix="$root/boost_1_70_0" cxxflags="-std=c++17 -fPIC" install
    popd
    unset BOOST_BUILD_PATH
    rm boost_1_70_0.tar.gz
  fi
fi
