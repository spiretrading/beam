#!/bin/bash
let cores="`grep -c "processor" < /proc/cpuinfo`"
username=$(echo ${SUDO_USER:-${USER}})

if [ ! -d "cppunit-1.14.0" ]; then
  sudo -u $username wget http://dev-www.libreoffice.org/src/cppunit-1.14.0.tar.gz --no-check-certificate
  if [ -f cppunit-1.14.0.tar.gz ]; then
    sudo -u $username gzip -d -c cppunit-1.14.0.tar.gz | sudo -u $username tar -x
    sudo -u $username chmod -R g-w cppunit-1.14.0
    sudo -u $username chmod -R o-w cppunit-1.14.0
    pushd cppunit-1.14.0
    sudo -u $username touch configure.new
    cat configure | sed "s/\/\* automatically generated \*\//\$ac_prefix_conf_INP/" > configure.new
    sudo -u $username mv configure.new configure
    sudo -u $username chmod a+x configure
    sudo -u $username ./configure LDFLAGS='-ldl'
    sudo -u $username make -j $cores
    make install
    popd
    rm -f cppunit-1.14.0.tar.gz
  fi
fi
if [ ! -d "cryptopp565" ]; then
  sudo -u $username wget https://github.com/weidai11/cryptopp/archive/b0f3b8ce1761e7ab9a3ead46fb7403fb38dd3723.zip -O cryptopp565.zip --no-check-certificate
  if [ -f cryptopp565.zip ]; then
    sudo -u $username unzip cryptopp565.zip
    sudo -u $username mv cryptopp-b0f3b8ce1761e7ab9a3ead46fb7403fb38dd3723 cryptopp565
    pushd cryptopp565
    sudo -u $username chmod +x GNUmakefile
    sudo -u $username make -j $cores
    make install
    popd
    rm -f cryptopp565.zip
  fi
fi
if [ ! -d "zlib-1.2.8" ]; then
  sudo -u $username wget https://github.com/madler/zlib/archive/v1.2.8.zip --no-check-certificate
  if [ -f v1.2.8.zip ]; then
    sudo -u $username unzip v1.2.8.zip
    chown -R $username:$username zlib-1.2.8
    pushd zlib-1.2.8
    export CFLAGS="-fPIC"
    sudo -E -u $username cmake -G "Unix Makefiles" -DAMD64=ON
    sudo -E -u $username make -j $cores
    make install
    unset CFLAGS
    popd
    rm -f v1.2.8.zip
  fi
fi
if [ ! -d "mysql-connector-c-6.1.11" ]; then
  sudo -u $username wget http://dev.mysql.com/get/Downloads/Connector-C/mysql-connector-c-6.1.11-src.tar.gz
  if [ -f mysql-connector-c-6.1.11-src.tar.gz ]; then
    sudo -u $username gzip -d -c mysql-connector-c-6.1.11-src.tar.gz | sudo -u $username tar -x
    sudo -u $username mv mysql-connector-c-6.1.11-src mysql-connector-c-6.1.11
    chown -R $username:$username mysql-connector-c-6.1.11
    pushd mysql-connector-c-6.1.11
    sudo -u $username cmake -G "Unix Makefiles"
    sudo -u $username make -j $cores
    make install
    popd
    rm -f mysql-connector-c-6.1.11-src.tar.gz
  fi
fi
if [ ! -d "mysql++-3.2.3" ]; then
  sudo -u $username wget https://tangentsoft.com/mysqlpp/releases/mysql++-3.2.3.tar.gz --no-check-certificate
  if [ -f mysql++-3.2.3.tar.gz ]; then
    sudo -u $username gzip -d -c mysql++-3.2.3.tar.gz | sudo -u $username tar -x
    pushd mysql++-3.2.3
    sudo -u $username ./configure
    sudo -u $username make -j $cores
    make install
    popd
    rm -f mysql++-3.2.3.tar.gz
  fi
fi
if [ ! -d "yaml-cpp" ]; then
  sudo -u $username wget https://github.com/jbeder/yaml-cpp/archive/release-0.2.7.zip --no-check-certificate
  if [ -f release-0.2.7.zip ]; then
    sudo -u $username unzip release-0.2.7.zip
    sudo -u $username mv yaml-cpp-release-0.2.7 yaml-cpp
    pushd yaml-cpp/include/yaml-cpp
    sudo -u $username touch noncopyable.h.new
    sudo -u $username head -7 noncopyable.h > noncopyable.h.new
    sudo -u $username printf "#include <stdlib.h>" >> noncopyable.h.new
    sudo -u $username tail -n+7 noncopyable.h >> noncopyable.h.new
    sudo -u $username mv noncopyable.h.new noncopyable.h
    popd
    pushd yaml-cpp
    sudo -u $username mkdir build
    popd
    pushd yaml-cpp/build
    export CFLAGS="-fPIC"
    export CXXFLAGS="-fPIC"
    sudo -E -u $username cmake ..
    sudo -E -u $username make -j $cores
    make install
    unset CFLAGS
    unset CXXFLAGS
    popd
    rm -f release-0.2.7.zip
  fi
fi
if [ ! -d "tclap-1.2.1" ]; then
  sudo -u $username wget "https://downloads.sourceforge.net/project/tclap/tclap-1.2.1.tar.gz?r=&ts=1309913922&use_mirror=superb-sea2" -O tclap-1.2.1.tar.gz --no-check-certificate
  if [ -f tclap-1.2.1.tar.gz ]; then
    sudo -u $username gzip -d -c tclap-1.2.1.tar.gz | sudo -u $username tar -x
    chown -R $username:$username tclap-1.2.1
    sudo -u $username chmod -R g-w tclap-1.2.1
    sudo -u $username chmod -R o-w tclap-1.2.1
    pushd tclap-1.2.1
    sudo -u $username ./configure
    sudo -u $username make -j $cores
    make install
    popd
    rm -f tclap-1.2.1.tar.gz
  fi
fi
if [ ! -d "openssl-1.0.2g" ]; then
  sudo -u $username wget --no-check-certificate https://www.openssl.org/source/openssl-1.0.2g.tar.gz
  if [ -f openssl-1.0.2g.tar.gz ]; then
    sudo -u $username gzip -d -c openssl-1.0.2g.tar.gz | sudo -u $username tar -x
    chown -R $username:$username openssl-1.0.2g
    pushd openssl-1.0.2g
    export LDFLAGS=-ldl
    sudo -E -u $username ./config no-shared threads -fPIC -ldl
    sudo -E -u $username make
    sudo -E -u $username make test
    make install
    unset LDFLAGS
    popd
    rm openssl-1.0.2g.tar.gz
  fi
fi
if [ ! -d "boost_1_66_0" ]; then
  sudo -u $username wget https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.gz -O boost_1_66_0.tar.gz --no-check-certificate
  if [ -f boost_1_66_0.tar.gz ]; then
    sudo -u $username tar xvf boost_1_66_0.tar.gz
    pushd boost_1_66_0
    export BOOST_BUILD_PATH=$(pwd)
    sudo -u $username cp tools/build/example/user-config.jam .
    sudo -u $username printf "using python : 3.5 : /usr/bin/python3 : /usr/include/python3.5 : /usr/lib ;\n" >> user-config.jam
    sudo -u $username ./bootstrap.sh --with-python=/usr/bin/python3 --with-python-version=3.5 --with-python-root=/usr/local/lib/python3.5
    sudo -u $username ./b2 -j$cores cxxflags="-std=c++14 -fPIC" stage
    ./b2 install
    popd
    unset BOOST_BUILD_PATH
    rm -f boost_1_66_0.tar.gz
  fi
fi
if [ ! -d "lua-5.3.1" ]; then
  sudo -u $username wget http://www.lua.org/ftp/lua-5.3.1.tar.gz
  if [ -f lua-5.3.1.tar.gz ]; then
    sudo -u $username gzip -d -c lua-5.3.1.tar.gz | sudo -u $username tar -x
    chown -R $username:$username lua-5.3.1
    pushd lua-5.3.1
    sudo -u $username make -j $cores linux
    make linux install
    popd
    rm -f lua-5.3.1.tar.gz
  fi
fi
if [ ! -d "mysql-connector-python-2.1.5" ]; then
  sudo -u $username wget https://dev.mysql.com/get/Downloads/Connector-Python/mysql-connector-python-2.1.5.zip --no-check-certificate
  sudo -u $username unzip mysql-connector-python-2.1.5.zip
  pushd mysql-connector-python-2.1.5
  sudo -u $username python3 setup.py build
  python3 setup.py install
  popd
  rm -f mysql-connector-python-2.1.5.zip
fi

sudo -u $username pip3 install Sphinx
sudo -u $username pip3 install sphinx-jsondomain
sudo -u $username pip3 install sphinx_rtd_theme
sudo -u $username pip3 install sphinxcontrib-httpdomain
