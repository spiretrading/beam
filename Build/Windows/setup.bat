SETLOCAL
SET LIBRARY_DIR=%cd%

if exist cppunit-1.14.0 goto end_cppunit_setup
  wget http://dev-www.libreoffice.org/src/cppunit-1.14.0.tar.gz --no-check-certificate
  if not exist cppunit-1.14.0.tar.gz goto end_cppunit_setup
    gzip -d -c cppunit-1.14.0.tar.gz | tar -x
    pushd cppunit-1.14.0\src\cppunit
    msbuild cppunit.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=Debug
    msbuild cppunit.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=Release
    popd
    rm cppunit-1.14.0.tar.gz
:end_cppunit_setup

if exist cryptopp610 goto end_cryptopp_setup
  wget http://www.cryptopp.com/cryptopp610.zip --no-check-certificate
  if not exist cryptopp610.zip goto end_cryptopp_setup
    mkdir cryptopp610
    pushd cryptopp610
    unzip ../cryptopp610.zip
    devenv /Upgrade cryptlib.vcproj
    cat cryptlib.vcproj | sed "s/WholeProgramOptimization=\"1\"/WholeProgramOptimization=\"0\"/" | sed "s/WholeProgramOptimization=\"true\"/WholeProgramOptimization=\"false\"/" > cryptlib.vcproj.new
    mv cryptlib.vcproj.new cryptlib.vcproj
    cat cryptlib.vcxproj | sed "s/<WholeProgramOptimization>true<\/WholeProgramOptimization>/<WholeProgramOptimization>false<\/WholeProgramOptimization>/" > cryptlib.vcxproj.new
    mv cryptlib.vcxproj.new cryptlib.vcxproj
    cat cryptlib.vcproj | sed "s/RuntimeLibrary=\"0\"/RuntimeLibrary=\"2\"/" | sed "s/RuntimeLibrary=\"1\"/RuntimeLibrary=\"3\"/" > cryptlib.vcproj.new
    mv cryptlib.vcproj.new cryptlib.vcproj
    cat cryptlib.vcxproj | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > cryptlib.vcxproj.new
    mv cryptlib.vcxproj.new cryptlib.vcxproj
    devenv cryptlib.vcxproj /useenv /Build "Debug"
    devenv cryptlib.vcxproj /useenv /Build "Release"
    mkdir include
    pushd include
    mkdir cryptopp
    cp ../*.h cryptopp
    popd
    popd
    rm cryptopp610.zip
:end_cryptopp_setup

if exist zlib-1.2.11 goto end_zlib_setup
  git clone --branch v1.2.11 https://github.com/madler/zlib.git zlib-1.2.11
  if not exist zlib-1.2.11 goto end_zlib_setup
    pushd zlib-1.2.11\contrib\vstudio\vc14
    cat zlibstat.vcxproj | sed "s/ZLIB_WINAPI;//" | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > zlibstat.vcxproj.new
    mv zlibstat.vcxproj.new zlibstat.vcxproj
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=Debug
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=ReleaseWithoutAsm
    popd
:end_zlib_setup

if exist mysql-connector-c-6.1.11-win32 goto end_mysqlconnector_setup
  wget http://dev.mysql.com/get/Downloads/Connector-C/mysql-connector-c-6.1.11-win32.zip --no-check-certificate
  if not exist mysql-connector-c-6.1.11-win32.zip goto end_mysqlconnector_setup
    unzip mysql-connector-c-6.1.11-win32.zip
    rm mysql-connector-c-6.1.11-win32.zip
:end_mysqlconnector_setup

if exist mysql++-3.2.3 goto end_mysqlpp_setup
  git clone https://github.com/eidolonsystems/mysqlpp mysql++-3.2.3
  if not exist mysql++-3.2.3 goto end_mysqlpp_setup
    pushd mysql++-3.2.3\vc2005
    SET CL=/I..\..\mysql-connector-c-6.1.11-win32\include
    devenv mysql++_mysqlpp.vcxproj /useenv /Build Debug
    devenv mysql++_mysqlpp.vcxproj /useenv /Build Release
    SET CL=
    popd
:end_mysqlpp_setup

if exist yaml-cpp goto end_yaml_setup
  git clone https://github.com/jbeder/yaml-cpp.git yaml-cpp
  if not exist yaml-cpp goto end_yaml_setup
    pushd yaml-cpp
    git checkout 0f9a586ca1dc29c2ecb8dd715a315b93e3f40f79
    mkdir build
    pushd build
    cmake -G "Visual Studio 15 2017" ..
    cmake --build . --target ALL_BUILD --config Debug
    cmake --build . --target ALL_BUILD --config Release
    popd
    popd
:end_yaml_setup

if exist tclap-1.2.1 goto end_tclap_setup
  wget "http://downloads.sourceforge.net/project/tclap/tclap-1.2.1.tar.gz?r=&ts=1309913922&use_mirror=superb-sea2" -O tclap-1.2.1.tar.gz --no-check-certificate
  if not exist tclap-1.2.1.tar.gz goto end_tclap_setup
    gzip -d -c tclap-1.2.1.tar.gz | tar -x
    rm tclap-1.2.1.tar.gz
:end_tclap_setup

if exist openssl-1.0.2g goto end_openssl_setup
  wget ftp://ftp.openssl.org/source/old/1.0.2/openssl-1.0.2g.tar.gz
  if not exist openssl-1.0.2g.tar.gz goto end_openssl_setup
    gzip -d -c openssl-1.0.2g.tar.gz | tar -x
    pushd openssl-1.0.2g
    perl Configure VC-WIN32 no-asm --prefix=%~dp0../../../openssl-1.0.2g
    CALL ./ms/do_ms
    nmake -f ./ms/nt.mak
    popd
    rm openssl-1.0.2g.tar.gz
:end_openssl_setup

if exist boost_1_67_0 goto end_boost_setup
  wget --no-check-certificate https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.zip -O boost_1_67_0.zip
  if not exist boost_1_67_0.zip goto end_boost_setup
    unzip boost_1_67_0.zip
    pushd boost_1_67_0
    wget --no-check-certificate https://www.boost.org/patches/1_67_0/0003-Python-Fix-auto-linking-logic-Windows-only.patch
    git apply 0003-Python-Fix-auto-linking-logic-Windows-only.patch
    CALL bootstrap.bat vc141
    if "%NUMBER_OF_PROCESSORS%" == "" (
      SET BJAM_PROCESSORS=
    ) else (
      SET BJAM_PROCESSORS="-j"%NUMBER_OF_PROCESSORS%
    )
    b2 %BJAM_PROCESSORS% --without-context --prefix=%cd% --build-type=complete toolset=msvc-14.1 link=static,shared runtime-link=shared install
    b2 %BJAM_PROCESSORS% --with-context --prefix=%cd% --build-type=complete toolset=msvc-14.1 link=static runtime-link=shared install
    popd
    rm boost_1_67_0.zip
:end_boost_setup

if exist lua-5.3.1 goto end_lua_setup
  wget --no-check-certificate http://www.lua.org/ftp/lua-5.3.1.tar.gz
  if not exist lua-5.3.1.tar.gz goto end_lua_setup
    gzip -d -c lua-5.3.1.tar.gz | tar -x
    pushd lua-5.3.1\src
    cp %~dp0/lua_cmakelists.txt CMakeLists.txt
    cmake -G "Visual Studio 15 2017" .
    cmake --build . --target ALL_BUILD --config Debug
    cmake --build . --target ALL_BUILD --config Release
    popd
    rm lua-5.3.1.tar.gz
:end_lua_setup

if exist mysql-connector-python-2.1.5 goto end_mysql_python
  wget --no-check-certificate https://dev.mysql.com/get/Downloads/Connector-Python/mysql-connector-python-2.1.5.zip
  if not exist mysql-connector-python-2.1.5.zip goto end_mysql_python
    unzip mysql-connector-python-2.1.5.zip
    pushd mysql-connector-python-2.1.5
    python setup.py install
    popd
    rm -f mysql-connector-python-2.1.5.zip
:end_mysql_python

ENDLOCAL
