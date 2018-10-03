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
if [ ! -d "zlib-1.2.11" ]; then
  sudo -u $username wget https://github.com/madler/zlib/archive/v1.2.11.zip --no-check-certificate
  if [ -f v1.2.11.zip ]; then
    sudo -u $username unzip v1.2.11.zip
    chown -R $username:$username zlib-1.2.11
    pushd zlib-1.2.11
    export CFLAGS="-fPIC"
    sudo -E -u $username cmake -G "Unix Makefiles"
    sudo -E -u $username make -j $cores
    make install
    unset CFLAGS
    popd
    rm -f v1.2.11.zip
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
  sudo -u $username git clone https://github.com/eidolonsystems/mysqlpp mysql++-3.2.3
  if [ -d mysql++-3.2.3 ]; then
    pushd mysql++-3.2.3
    sudo -u $username ./configure
    sudo -u $username make -j $cores
    make install
    popd
  fi
fi
if [ ! -d "yaml-cpp" ]; then
  sudo -u $username git clone --branch yaml-cpp-0.6.2 https://github.com/jbeder/yaml-cpp.git yaml-cpp
  if [ -d "yaml-cpp" ]; then
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
  fi
fi
if [ ! -d "sqlite" ]; then
  sudo -u $username wget --no-check-certificate https://www.sqlite.org/2018/sqlite-amalgamation-3230100.zip
  if [ -f sqlite-amalgamation-3230100.zip ]; then
    sudo -u $username unzip sqlite-amalgamation-3230100.zip
    sudo -u $username mv sqlite-amalgamation-3230100 sqlite
    pushd sqlite
    sudo -u $username gcc -c -O2 -o sqlite3.lib -DSQLITE_USE_URI=1 -fPIC sqlite3.c
    popd
    rm sqlite-amalgamation-3230100.zip
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
if [ ! -d "boost_1_67_0" ]; then
  sudo -u $username wget https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.gz -O boost_1_67_0.tar.gz --no-check-certificate
  if [ -f boost_1_67_0.tar.gz ]; then
    sudo -u $username tar xvf boost_1_67_0.tar.gz
    pushd boost_1_67_0
    export BOOST_BUILD_PATH=$(pwd)
    sudo -u $username cp tools/build/example/user-config.jam .
    sudo -u $username printf "using python : 3.6 : /usr/bin/python3 : /usr/include/python3.6 : /usr/lib ;\n" >> user-config.jam
    sudo -u $username ./bootstrap.sh --with-python=/usr/bin/python3 --with-python-version=3.6 --with-python-root=/usr/local/lib/python3.6
    sudo -u $username ./b2 -j$cores cxxflags="-std=c++17 -fPIC" stage
    ./b2 install
    popd
    unset BOOST_BUILD_PATH
    rm -f boost_1_67_0.tar.gz
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
if [ ! -d "viper" ]; then
  sudo -u $username git clone https://www.github.com/eidolonsystems/viper
fi
if [ -d "viper" ]; then
  viper_commit="0631eff5a0a36d77bc45da1b0118dd49ea22953b"
  pushd viper
  commit="`git log -1 | head -1 | awk '{ print $2 }'`"
  if [ "$commit" != "$viper_commit" ]; then
    sudo -u $username git checkout master
    sudo -u $username git pull
    sudo -u $username git checkout "$viper_commit"
  fi
  sudo -E -u $username cmake -G "Unix Makefiles"
  popd
fi

sudo -u $username pip3 install Sphinx
sudo -u $username pip3 install sphinx-jsondomain
sudo -u $username pip3 install sphinx_rtd_theme
sudo -u $username pip3 install sphinxcontrib-httpdomain
