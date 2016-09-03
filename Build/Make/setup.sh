#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"

if [ ! -d "cppunit-1.12.1" ]; then
  wget http://downloads.sourceforge.net/cppunit/cppunit-1.12.1.tar.gz
  gzip -d -c cppunit-1.12.1.tar.gz | tar -x
  cd cppunit-1.12.1
  cat configure | sed "s/\/\* automatically generated \*\//\$ac_prefix_conf_INP/" > configure.new
  mv configure.new configure
  chmod +rwx configure
  ./configure LDFLAGS='-ldl'
  make -j $cores
  make install
  cd ..
  rm -f cppunit-1.12.1.tar.gz
fi
if [ ! -d "cryptopp562" ]; then
  wget http://www.cryptopp.com/cryptopp562.zip
  mkdir cryptopp562
  cd cryptopp562
  unzip ../cryptopp562.zip
  cat GNUmakefile | sed "s/# CXXFLAGS += -fPIC/CXXFLAGS += -fPIC/" > GNUmakefile.new
  mv GNUmakefile.new GNUmakefile
  make -j $cores
  make install
  cd ..
  rm -f cryptopp562.zip
fi
if [ ! -d "zlib-1.2.8" ]; then
  wget http://zlib.net/zlib-1.2.8.tar.gz
  gzip -d -c zlib-1.2.8.tar.gz | tar -x
  cd zlib-1.2.8
  export CFLAGS="-fPIC"
  cmake -G "Unix Makefiles" -DAMD64=ON
  make -j $cores
  make install
  unset CFLAGS
  cd ..
  rm -f zlib-1.2.8.tar.gz
fi
if [ ! -d "mysql-connector-c-6.1.6" ]; then
  wget http://dev.mysql.com/get/Downloads/Connector-C/mysql-connector-c-6.1.6-src.tar.gz
  gzip -d -c mysql-connector-c-6.1.6-src.tar.gz | tar -x
  mv mysql-connector-c-6.1.6-src mysql-connector-c-6.1.6
  cd mysql-connector-c-6.1.6
  cmake -G "Unix Makefiles"
  make -j $cores
  make install
  cd ..
  rm -f mysql-connector-c-6.1.6-src.tar.gz
fi
if [ ! -d "mysql++-3.2.2" ]; then
  wget http://tangentsoft.net/mysql++/releases/mysql++-3.2.2.tar.gz
  gzip -d -c mysql++-3.2.2.tar.gz | tar -x
  cd mysql++-3.2.2
  ./configure
  make -j $cores
  make install
  cd ..
  rm -f mysql++-3.2.2.tar.gz
fi
if [ ! -d "yaml-cpp" ]; then
  wget https://github.com/jbeder/yaml-cpp/archive/release-0.2.7.zip
  unzip release-0.2.7.zip
  mv yaml-cpp-release-0.2.7 yaml-cpp
  cd yaml-cpp/include/yaml-cpp
  head -7 noncopyable.h > noncopyable.h.new
  printf "#include <stdlib.h>" >> noncopyable.h.new
  tail -n+7 noncopyable.h >> noncopyable.h.new
  mv noncopyable.h.new noncopyable.h
  cd ../../
  mkdir build
  cd build
  export CFLAGS="-fPIC"
  export CXXFLAGS="-fPIC"
  cmake ..
  make -j $cores
  make install
  unset CFLAGS
  unset CXXFLAGS
  cd ../..
  rm -f release-0.2.7.zip
fi
if [ ! -d "tclap-1.2.1" ]; then
  wget "https://downloads.sourceforge.net/project/tclap/tclap-1.2.1.tar.gz?r=&ts=1309913922&use_mirror=superb-sea2" -O tclap-1.2.1.tar.gz --no-check-certificate
  gzip -d -c tclap-1.2.1.tar.gz | tar -x
  cd tclap-1.2.1
  ./configure
  make -j $cores
  make install
  cd ..
  rm -f tclap-1.2.1.tar.gz
fi
if [ ! -d "openssl-1.0.2g" ]; then
  wget --no-check-certificate https://www.openssl.org/source/openssl-1.0.2g.tar.gz
  gzip -d -c openssl-1.0.2g.tar.gz | tar -x
  cd openssl-1.0.2g
  export LDFLAGS=-ldl
  ./config no-shared threads -fPIC -ldl
  make
  make test
  make install
  unset LDFLAGS
  cd ..
  rm openssl-1.0.2g.tar.gz
fi
if [ ! -d "boost_1_61_0" ]; then
  wget http://sourceforge.net/projects/boost/files/boost/1.61.0/boost_1_61_0.tar.gz/download -O boost_1_61_0.tar.gz
  tar xvf boost_1_61_0.tar.gz
  cd boost_1_61_0
  ./bootstrap.sh
  ./bjam -j$cores cxxflags="-std=c++14 -fPIC" install
  cd ..
  rm -f boost_1_61_0.tar.gz
fi
if [ ! -d "lua-5.3.1" ]; then
  wget http://www.lua.org/ftp/lua-5.3.1.tar.gz
  gzip -d -c lua-5.3.1.tar.gz | tar -x
  cd lua-5.3.1
  make -j $cores linux
  make linux install
  cd ..
  rm -f lua-5.3.1.tar.gz
fi
