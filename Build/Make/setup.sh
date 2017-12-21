#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"

if [ ! -d "cppunit-1.14.0" ]; then
  sudo -u $(logname) wget http://dev-www.libreoffice.org/src/cppunit-1.14.0.tar.gz --no-check-certificate
  if [ -f cppunit-1.14.0.tar.gz ]; then
    sudo -u $(logname) gzip -d -c cppunit-1.14.0.tar.gz | sudo -u $(logname) tar -x
    sudo -u $(logname) chmod -R g-w cppunit-1.14.0
    sudo -u $(logname) chmod -R o-w cppunit-1.14.0
    pushd cppunit-1.14.0
    sudo -u $(logname) touch configure.new
    cat configure | sed "s/\/\* automatically generated \*\//\$ac_prefix_conf_INP/" > configure.new
    sudo -u $(logname) mv configure.new configure
    sudo -u $(logname) chmod a+x configure
    sudo -u $(logname) ./configure LDFLAGS='-ldl'
    sudo -u $(logname) make -j $cores
    make install
    popd
    rm -f cppunit-1.14.0.tar.gz
  fi
fi
if [ ! -d "cryptopp565" ]; then
  sudo -u $(logname) wget https://github.com/weidai11/cryptopp/archive/b0f3b8ce1761e7ab9a3ead46fb7403fb38dd3723.zip -O cryptopp565.zip --no-check-certificate
  if [ -f cryptopp565.zip ]; then
    sudo -u $(logname) unzip cryptopp565.zip
    sudo -u $(logname) mv cryptopp-b0f3b8ce1761e7ab9a3ead46fb7403fb38dd3723 cryptopp565
    pushd cryptopp565
    sudo -u $(logname) chmod +x GNUmakefile
    sudo -u $(logname) make -j $cores
    make install
    popd
    rm -f cryptopp565.zip
  fi
fi
if [ ! -d "zlib-1.2.8" ]; then
  sudo -u $(logname) wget https://github.com/madler/zlib/archive/v1.2.8.zip --no-check-certificate
  if [ -f v1.2.8.zip ]; then
    sudo -u $(logname) unzip v1.2.8.zip
    chown -R $(logname):$(logname) zlib-1.2.8
    pushd zlib-1.2.8
    export CFLAGS="-fPIC"
    sudo -E -u $(logname) cmake -G "Unix Makefiles" -DAMD64=ON
    sudo -E -u $(logname) make -j $cores
    make install
    unset CFLAGS
    popd
    rm -f v1.2.8.zip
  fi
fi
if [ ! -d "mysql-connector-c-6.1.11" ]; then
  sudo -u $(logname) wget http://dev.mysql.com/get/Downloads/Connector-C/mysql-connector-c-6.1.11-src.tar.gz
  if [ -f mysql-connector-c-6.1.11-src.tar.gz ]; then
    sudo -u $(logname) gzip -d -c mysql-connector-c-6.1.11-src.tar.gz | sudo -u $(logname) tar -x
    sudo -u $(logname) mv mysql-connector-c-6.1.11-src mysql-connector-c-6.1.11
    chown -R $(logname):$(logname) mysql-connector-c-6.1.11
    pushd mysql-connector-c-6.1.11
    sudo -u $(logname) cmake -G "Unix Makefiles"
    sudo -u $(logname) make -j $cores
    make install
    popd
    rm -f mysql-connector-c-6.1.11-src.tar.gz
  fi
fi
if [ ! -d "mysql++-3.2.3" ]; then
  sudo -u $(logname) wget https://tangentsoft.com/mysqlpp/releases/mysql++-3.2.3.tar.gz --no-check-certificate
  if [ -f mysql++-3.2.3.tar.gz ]; then
    sudo -u $(logname) gzip -d -c mysql++-3.2.3.tar.gz | sudo -u $(logname) tar -x
    pushd mysql++-3.2.3
    sudo -u $(logname) ./configure
    sudo -u $(logname) make -j $cores
    make install
    popd
    rm -f mysql++-3.2.3.tar.gz
  fi
fi
if [ ! -d "yaml-cpp" ]; then
  sudo -u $(logname) wget https://github.com/jbeder/yaml-cpp/archive/release-0.2.7.zip --no-check-certificate
  if [ -f release-0.2.7.zip ]; then
    sudo -u $(logname) unzip release-0.2.7.zip
    sudo -u $(logname) mv yaml-cpp-release-0.2.7 yaml-cpp
    pushd yaml-cpp/include/yaml-cpp
    sudo -u $(logname) touch noncopyable.h.new
    sudo -u $(logname) head -7 noncopyable.h > noncopyable.h.new
    sudo -u $(logname) printf "#include <stdlib.h>" >> noncopyable.h.new
    sudo -u $(logname) tail -n+7 noncopyable.h >> noncopyable.h.new
    sudo -u $(logname) mv noncopyable.h.new noncopyable.h
    popd
    pushd yaml-cpp
    sudo -u $(logname) mkdir build
    popd
    pushd yaml-cpp/build
    export CFLAGS="-fPIC"
    export CXXFLAGS="-fPIC"
    sudo -E -u $(logname) cmake ..
    sudo -E -u $(logname) make -j $cores
    make install
    unset CFLAGS
    unset CXXFLAGS
    popd
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
    pushd tclap-1.2.1
    sudo -u $(logname) ./configure
    sudo -u $(logname) make -j $cores
    make install
    popd
    rm -f tclap-1.2.1.tar.gz
  fi
fi
if [ ! -d "openssl-1.0.2g" ]; then
  sudo -u $(logname) wget --no-check-certificate https://www.openssl.org/source/openssl-1.0.2g.tar.gz
  if [ -f openssl-1.0.2g.tar.gz ]; then
    sudo -u $(logname) gzip -d -c openssl-1.0.2g.tar.gz | sudo -u $(logname) tar -x
    chown -R $(logname):$(logname) openssl-1.0.2g
    pushd openssl-1.0.2g
    export LDFLAGS=-ldl
    sudo -E -u $(logname) ./config no-shared threads -fPIC -ldl
    sudo -E -u $(logname) make
    sudo -E -u $(logname) make test
    make install
    unset LDFLAGS
    popd
    rm openssl-1.0.2g.tar.gz
  fi
fi
if [ ! -d "boost_1_66_0" ]; then
  sudo -u $(logname) wget https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz -O boost_1_66_0.tar.gz --no-check-certificate
  if [ -f boost_1_66_0.tar.gz ]; then
    sudo -u $(logname) tar xvf boost_1_66_0.tar.gz
    pushd boost_1_66_0
    sudo -u $(logname) ./bootstrap.sh
    sudo -u $(logname) ./bjam -j$cores cxxflags="-std=c++14 -fPIC" stage
    ./bjam install
    popd
    rm -f boost_1_66_0.tar.gz
  fi
fi
if [ ! -d "lua-5.3.1" ]; then
  sudo -u $(logname) wget http://www.lua.org/ftp/lua-5.3.1.tar.gz
  if [ -f lua-5.3.1.tar.gz ]; then
    sudo -u $(logname) gzip -d -c lua-5.3.1.tar.gz | sudo -u $(logname) tar -x
    chown -R $(logname):$(logname) lua-5.3.1
    pushd lua-5.3.1
    sudo -u $(logname) make -j $cores linux
    make linux install
    popd
    rm -f lua-5.3.1.tar.gz
  fi
fi
if [ ! -d "mysql-connector-python-2.1.5" ]; then
  sudo -u $(logname) wget https://dev.mysql.com/get/Downloads/Connector-Python/mysql-connector-python-2.1.5.zip --no-check-certificate
  sudo -u $(logname) unzip mysql-connector-python-2.1.5.zip
  pushd mysql-connector-python-2.1.5
  sudo -u $(logname) python setup.py build
  python setup.py install
  popd
  rm -f mysql-connector-python-2.1.5.zip
fi

sudo -u $(logname) pip install Sphinx
sudo -u $(logname) pip install sphinx-jsondomain
sudo -u $(logname) pip install sphinx_rtd_theme
sudo pip install sphinxcontrib-httpdomain
