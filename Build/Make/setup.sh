#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"

if [ ! -d "cppunit-1.12.1" ]; then
  sudo -u $(logname) wget http://downloads.sourceforge.net/cppunit/cppunit-1.12.1.tar.gz
  if [ -f cppunit-1.12.1.tar.gz ]; then
    sudo -u $(logname) gzip -d -c cppunit-1.12.1.tar.gz | sudo -u $(logname) tar -x
    sudo -u $(logname) chmod -R g-w cppunit-1.12.1
    sudo -u $(logname) chmod -R o-w cppunit-1.12.1
    cd cppunit-1.12.1
    sudo -u $(logname) touch configure.new
    cat configure | sed "s/\/\* automatically generated \*\//\$ac_prefix_conf_INP/" > configure.new
    sudo -u $(logname) mv configure.new configure
    sudo -u $(logname) chmod a+x configure
    sudo -u $(logname) ./configure LDFLAGS='-ldl'
    sudo -u $(logname) make -j $cores
    make install
    cd ..
    rm -f cppunit-1.12.1.tar.gz
  fi
fi
if [ ! -d "cryptopp565" ]; then
  sudo -u $(logname) wget http://www.cryptopp.com/cryptopp565.zip
  if [ -f cryptopp565.zip ]; then
    sudo -u $(logname) mkdir cryptopp565
    cd cryptopp565
    sudo -u $(logname) unzip ../cryptopp565.zip
    sudo -u $(logname) touch GNUmakefile.new
    cat GNUmakefile | sed "s/# CXXFLAGS += -fPIC/CXXFLAGS += -fPIC/" > GNUmakefile.new
    sudo -u $(logname) mv GNUmakefile.new GNUmakefile
    sudo -u $(logname) chmod +x GNUmakefile
    sudo -u $(logname) make -j $cores
    make install
    cd ..
    rm -f cryptopp565.zip
  fi
fi
if [ ! -d "zlib-1.2.11" ]; then
  sudo -u $(logname) wget http://www.zlib.net/zlib-1.2.11.tar.gz
  if [ -f zlib-1.2.11.tar.gz ]; then
    sudo -u $(logname) gzip -d -c zlib-1.2.11.tar.gz | sudo -u $(logname) tar -x
    chown -R $(logname):$(logname) zlib-1.2.11
    cd zlib-1.2.11
    export CFLAGS="-fPIC"
    sudo -E -u $(logname) cmake -G "Unix Makefiles" -DAMD64=ON
    sudo -E -u $(logname) make -j $cores
    make install
    unset CFLAGS
    cd ..
    rm -f zlib-1.2.11.tar.gz
  fi
fi
if [ ! -d "mysql-connector-c-6.1.6" ]; then
  sudo -u $(logname) wget http://dev.mysql.com/get/Downloads/Connector-C/mysql-connector-c-6.1.6-src.tar.gz
  if [ -f mysql-connector-c-6.1.6-src.tar.gz ]; then
    sudo -u $(logname) gzip -d -c mysql-connector-c-6.1.6-src.tar.gz | sudo -u $(logname) tar -x
    sudo -u $(logname) mv mysql-connector-c-6.1.6-src mysql-connector-c-6.1.6
    chown -R $(logname):$(logname) mysql-connector-c-6.1.6
    cd mysql-connector-c-6.1.6
    sudo -u $(logname) cmake -G "Unix Makefiles"
    sudo -u $(logname) make -j $cores
    make install
    cd ..
    rm -f mysql-connector-c-6.1.6-src.tar.gz
  fi
fi
if [ ! -d "mysql++-3.2.2" ]; then
  sudo -u $(logname) wget http://tangentsoft.net/mysql++/releases/mysql++-3.2.2.tar.gz
  if [ -f mysql++-3.2.2.tar.gz ]; then
    sudo -u $(logname) gzip -d -c mysql++-3.2.2.tar.gz | sudo -u $(logname) tar -x
    cd mysql++-3.2.2
    sudo -u $(logname) ./configure
    sudo -u $(logname) make -j $cores
    make install
    cd ..
    rm -f mysql++-3.2.2.tar.gz
  fi
fi
if [ ! -d "yaml-cpp" ]; then
  sudo -u $(logname) wget https://github.com/jbeder/yaml-cpp/archive/release-0.2.7.zip
  if [ -f release-0.2.7.zip ]; then
    sudo -u $(logname) unzip release-0.2.7.zip
    sudo -u $(logname) mv yaml-cpp-release-0.2.7 yaml-cpp
    cd yaml-cpp/include/yaml-cpp
    sudo -u $(logname) touch noncopyable.h.new
    sudo -u $(logname) head -7 noncopyable.h > noncopyable.h.new
    sudo -u $(logname) printf "#include <stdlib.h>" >> noncopyable.h.new
    sudo -u $(logname) tail -n+7 noncopyable.h >> noncopyable.h.new
    sudo -u $(logname) mv noncopyable.h.new noncopyable.h
    cd ../../
    sudo -u $(logname) mkdir build
    cd build
    export CFLAGS="-fPIC"
    export CXXFLAGS="-fPIC"
    sudo -E -u $(logname) cmake ..
    sudo -E -u $(logname) make -j $cores
    make install
    unset CFLAGS
    unset CXXFLAGS
    cd ../..
    rm -f release-0.2.7.zip
  fi
fi
if [ ! -d "tclap-1.2.1" ]; then
  sudo -u $(logname) wget "https://downloads.sourceforge.net/project/tclap/tclap-1.2.1.tar.gz?r=&ts=1309913922&use_mirror=superb-sea2" -O tclap-1.2.1.tar.gz --no-check-certificate
  if [ -f tclap-1.2.1.tar.gz ]; then
    sudo -u $(logname) gzip -d -c tclap-1.2.1.tar.gz | sudo -u $(logname) tar -x
    chown -R $(logname):$(logname) tclap-1.2.1
    sudo -u $(logname) chmod -R g-w tclap-1.2.1
    sudo -u $(logname) chmod -R o-w tclap-1.2.1
    cd tclap-1.2.1
    sudo -u $(logname) ./configure
    sudo -u $(logname) make -j $cores
    make install
    cd ..
    rm -f tclap-1.2.1.tar.gz
  fi
fi
if [ ! -d "openssl-1.0.2g" ]; then
  sudo -u $(logname) wget --no-check-certificate https://www.openssl.org/source/openssl-1.0.2g.tar.gz
  if [ -f openssl-1.0.2g.tar.gz ]; then
    sudo -u $(logname) gzip -d -c openssl-1.0.2g.tar.gz | sudo -u $(logname) tar -x
    chown -R $(logname):$(logname) openssl-1.0.2g
    cd openssl-1.0.2g
    export LDFLAGS=-ldl
    sudo -E -u $(logname) ./config no-shared threads -fPIC -ldl
    sudo -E -u $(logname) make
    sudo -E -u $(logname) make test
    make install
    unset LDFLAGS
    cd ..
    rm openssl-1.0.2g.tar.gz
  fi
fi
if [ ! -d "boost_1_61_0" ]; then
  sudo -u $(logname) wget http://sourceforge.net/projects/boost/files/boost/1.61.0/boost_1_61_0.tar.gz/download -O boost_1_61_0.tar.gz
  if [ -f boost_1_61_0.tar.gz ]; then
    sudo -u $(logname) tar xvf boost_1_61_0.tar.gz
    cd boost_1_61_0
    sudo -u $(logname) ./bootstrap.sh
    ./bjam -j$cores cxxflags="-std=c++14 -fPIC" install
    cd ..
    rm -f boost_1_61_0.tar.gz
  fi
fi
if [ ! -d "lua-5.3.1" ]; then
  sudo -u $(logname) wget http://www.lua.org/ftp/lua-5.3.1.tar.gz
  if [ -f lua-5.3.1.tar.gz ]; then
    sudo -u $(logname) gzip -d -c lua-5.3.1.tar.gz | sudo -u $(logname) tar -x
    chown -R $(logname):$(logname) lua-5.3.1
    cd lua-5.3.1
    sudo -u $(logname) make -j $cores linux
    make linux install
    cd ..
    rm -f lua-5.3.1.tar.gz
  fi
fi
