SETLOCAL
SET LIBRARY_DIR=%cd%

if exist cppunit-1.14.0 goto end_cppunit_setup
  wget http://dev-www.libreoffice.org/src/cppunit-1.14.0.tar.gz --no-check-certificate
  if not exist cppunit-1.14.0.tar.gz goto end_cppunit_setup
    gzip -d -c cppunit-1.14.0.tar.gz | tar -x
    cd cppunit-1.14.0/src/cppunit
    msbuild cppunit.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=Debug
    msbuild cppunit.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=Release
    cd ../../../
    rm cppunit-1.14.0.tar.gz
:end_cppunit_setup

if exist cryptopp565 goto end_cryptopp_setup
  wget https://github.com/weidai11/cryptopp/archive/b0f3b8ce1761e7ab9a3ead46fb7403fb38dd3723.zip -O cryptopp565.zip --no-check-certificate
  if not exist cryptopp565.zip goto end_cryptopp_setup
    unzip cryptopp565.zip
    mv cryptopp-b0f3b8ce1761e7ab9a3ead46fb7403fb38dd3723 cryptopp565
    cd cryptopp565
    cmake -G "Visual Studio 15 2017" -DCMAKE_INSTALL_PREFIX="%LIBRARY_DIR%\cryptopp565" .
    cat cryptlib.vcxproj | sed "s/<WholeProgramOptimization>true<\/WholeProgramOptimization>/<WholeProgramOptimization>false<\/WholeProgramOptimization>/" > cryptlib.vcxproj.new
    mv cryptlib.vcxproj.new cryptlib.vcxproj
    cat cryptlib.vcxproj | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > cryptlib.vcxproj.new
    mv cryptlib.vcxproj.new cryptlib.vcxproj
    devenv cryptlib.vcxproj /useenv /Build "Debug"
    devenv cryptlib.vcxproj /useenv /Build "Release"
    mkdir include
    cd include
    mkdir cryptopp
    cp ../*.h cryptopp
    cd ../../
    rm cryptopp565.zip
:end_cryptopp_setup

if exist zlib-1.2.11 goto end_zlib_setup
  wget https://github.com/madler/zlib/archive/v1.2.11.zip -O v1.2.11.zip --no-check-certificate
  if not exist v1.2.11.zip goto end_zlib_setup
    unzip v1.2.11.zip
    cd zlib-1.2.11/contrib/vstudio/vc14
    cat zlibstat.vcxproj | sed "s/ZLIB_WINAPI;//" | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > zlibstat.vcxproj.new
    mv zlibstat.vcxproj.new zlibstat.vcxproj
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=Debug
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=ReleaseWithoutAsm
    cd ../../../../
    rm v1.2.11.zip
:end_zlib_setup

if exist mysql-connector-c-6.1.11-win32 goto end_mysqlconnector_setup
  wget http://dev.mysql.com/get/Downloads/Connector-C/mysql-connector-c-6.1.11-win32.zip --no-check-certificate
  if not exist mysql-connector-c-6.1.11-win32.zip goto end_mysqlconnector_setup
    unzip mysql-connector-c-6.1.11-win32.zip
    rm mysql-connector-c-6.1.11-win32.zip
:end_mysqlconnector_setup

if exist mysql++-3.2.3 goto end_mysqlpp_setup
  wget https://tangentsoft.com/mysqlpp/releases/mysql++-3.2.3.tar.gz --no-check-certificate
  if not exist mysql++-3.2.3.tar.gz goto end_mysqlpp_setup
    gzip -d -c mysql++-3.2.3.tar.gz | tar -x
    cd mysql++-3.2.3/vc2005
    cat mysql++_mysqlpp.vcproj | sed "s/ConfigurationType=\"2\"/ConfigurationType=\"4\"/" | sed "s/_USRDLL;//" | sed "s/DLL_EXPORTS//" | sed "s/MYSQLPP_MAKING_DLL/MYSQLPP_NO_DLL/" > mysql++_mysqlpp.vcproj.new
    mv mysql++_mysqlpp.vcproj.new mysql++_mysqlpp.vcproj
    devenv /Upgrade mysql++_mysqlpp.vcproj
    SET CL=/I..\..\mysql-connector-c-6.1.11-win32\include
    devenv mysql++_mysqlpp.vcxproj /useenv /Build Debug
    devenv mysql++_mysqlpp.vcxproj /useenv /Build Release
    SET CL=
    cd ..
    mkdir include
    cd include
    mkdir mysql++
    cp ../lib/*.h mysql++
    cd ../../
    rm mysql++-3.2.3.tar.gz
:end_mysqlpp_setup

if exist yaml-cpp goto end_yaml_setup
  wget "https://github.com/jbeder/yaml-cpp/archive/release-0.2.7.zip" --no-check-certificate
  if not exist release-0.2.7 goto end_yaml_setup
    unzip release-0.2.7
    mv yaml-cpp-release-0.2.7 yaml-cpp
    cd yaml-cpp/include/yaml-cpp
    head -7 noncopyable.h > noncopyable.h.new
    printf "#include <stdlib.h>" >> noncopyable.h.new
    tail -n+7 noncopyable.h >> noncopyable.h.new
    mv noncopyable.h.new noncopyable.h
    cd ../../
    mkdir build
    cd build
    cmake -G "Visual Studio 15 2017" ..
    cmake --build . --target ALL_BUILD --config Debug
    cmake --build . --target ALL_BUILD --config Release
    cd ../..
    rm release-0.2.7
:end_yaml_setup

if exist tclap-1.2.1 goto end_tclap_setup
  wget "https://downloads.sourceforge.net/project/tclap/tclap-1.2.1.tar.gz?r=&ts=1309913922&use_mirror=superb-sea2" -O tclap-1.2.1.tar.gz --no-check-certificate
  if not exist tclap-1.2.1.tar.gz goto end_tclap_setup
    gzip -d -c tclap-1.2.1.tar.gz | tar -x
    rm tclap-1.2.1.tar.gz
:end_tclap_setup

if exist openssl-1.0.2g goto end_openssl_setup
  wget ftp://ftp.openssl.org/source/old/1.0.2/openssl-1.0.2g.tar.gz
  if not exist openssl-1.0.2g.tar.gz goto end_openssl_setup
    gzip -d -c openssl-1.0.2g.tar.gz | tar -x
    cd openssl-1.0.2g
    perl Configure VC-WIN32 no-asm --prefix=C:/Development/Libraries/openssl-1.0.2g
    CALL ./ms/do_ms
    nmake -f ./ms/nt.mak
    cd ..
    rm openssl-1.0.2g.tar.gz
:end_openssl_setup

if exist boost_1_66_0 goto end_boost_setup
  wget --no-check-certificate https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.zip -O boost_1_66_0.zip
  if not exist boost_1_66_0.zip goto end_boost_setup
    unzip boost_1_66_0.zip
    cd boost_1_66_0
    CALL bootstrap.bat vc141
    if "%NUMBER_OF_PROCESSORS%" == "" (
      SET BJAM_PROCESSORS=
    ) else (
      SET BJAM_PROCESSORS="-j"%NUMBER_OF_PROCESSORS%
    )
    b2 %BJAM_PROCESSORS% --without-context --prefix=%cd% --build-type=complete toolset=msvc-14.1 link=static,shared runtime-link=shared install
    b2 %BJAM_PROCESSORS% --with-context --prefix=%cd% --build-type=complete toolset=msvc-14.1 link=static runtime-link=shared install
    cd ..
    rm boost_1_66_0.zip
:end_boost_setup

if exist lua-5.3.1 goto end_lua_setup
  wget --no-check-certificate http://www.lua.org/ftp/lua-5.3.1.tar.gz
  if not exist lua-5.3.1.tar.gz goto end_lua_setup
    gzip -d -c lua-5.3.1.tar.gz | tar -x
    cd lua-5.3.1/src
    cp %~dp0/lua_cmakelists.txt CMakeLists.txt
    cmake -G "Visual Studio 15 2017" .
    cmake --build . --target ALL_BUILD --config Debug
    cmake --build . --target ALL_BUILD --config Release
    cd ../..
    rm lua-5.3.1.tar.gz
:end_lua_setup

if exist mysql-connector-python-2.1.5 goto end_mysql_python
  wget --no-check-certificate https://dev.mysql.com/get/Downloads/Connector-Python/mysql-connector-python-2.1.5.zip
  if not exist mysql-connector-python-2.1.5.zip goto end_mysql_python
    unzip mysql-connector-python-2.1.5.zip
    cd mysql-connector-python-2.1.5
    python setup.py install
    cd ..
    rm -f mysql-connector-python-2.1.5.zip
:end_mysql_python

ENDLOCAL
