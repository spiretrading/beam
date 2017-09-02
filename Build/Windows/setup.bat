SETLOCAL

if exist cppunit-1.12.1 goto end_cppunit_setup
  wget https://sourceforge.net/projects/cppunit/files/cppunit/1.12.1/cppunit-1.12.1.tar.gz/download --no-check-certificate
  if not exist cppunit-1.12.1.tar.gz goto end_cppunit_setup
    gzip -d -c cppunit-1.12.1.tar.gz | tar -x
    cd cppunit-1.12.1/src
    devenv /Upgrade CppUnitLibraries.dsw
    cd cppunit
    devenv cppunit.vcxproj /useenv /Build "Debug"
    cp Debug/cppunitd.lib ../../lib/cppunitd.lib
    devenv cppunit.vcxproj /useenv /Build "Release"
    cd ../../../
    rm cppunit-1.12.1.tar.gz
:end_cppunit_setup

if exist cryptopp565 goto end_cryptopp_setup
  wget http://www.cryptopp.com/cryptopp565.zip --no-check-certificate
  if not exist cryptopp565.zip goto end_cryptopp_setup
    mkdir cryptopp565
    cd cryptopp565
    unzip ../cryptopp565.zip
    devenv /Upgrade cryptlib.vcxproj
    cat cryptlib.vcxproj | sed "s/WholeProgramOptimization=\"1\"/WholeProgramOptimization=\"0\"/" | sed "s/WholeProgramOptimization=\"true\"/WholeProgramOptimization=\"false\"/" > cryptlib.vcxproj.new
    mv cryptlib.vcxproj.new cryptlib.vcxproj
    cat cryptlib.vcxproj | sed "s/<WholeProgramOptimization>true<\/WholeProgramOptimization>/<WholeProgramOptimization>false<\/WholeProgramOptimization>/" > cryptlib.vcxproj.new
    mv cryptlib.vcxproj.new cryptlib.vcxproj
    cat cryptlib.vcxproj | sed "s/RuntimeLibrary=\"0\"/RuntimeLibrary=\"2\"/" | sed "s/RuntimeLibrary=\"1\"/RuntimeLibrary=\"3\"/" > cryptlib.vcxproj.new
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

if exist zlib-1.2.8 goto end_zlib_setup
  wget https://github.com/madler/zlib/archive/v1.2.8.zip -O v1.2.8.zip --no-check-certificate
  if not exist v1.2.8.zip goto end_zlib_setup
    unzip v1.2.8.zip
    cd zlib-1.2.8/contrib/vstudio/vc9
    devenv /Upgrade zlibstat.vcproj
    cat zlibstat.vcxproj | sed "s/ZLIB_WINAPI;//" | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > zlibstat.vcxproj.new
    mv zlibstat.vcxproj.new zlibstat.vcxproj
    devenv zlibstat.vcxproj /useenv /Build "Debug"
    devenv zlibstat.vcxproj /useenv /Build "ReleaseWithoutAsm"
    cd ../../../../
    rm v1.2.8.zip
:end_zlib_setup

if exist mysql-connector-c-6.1.6-win32 goto end_mysqlconnector_setup
  wget http://dev.mysql.com/get/Downloads/Connector-C/mysql-connector-c-6.1.6-win32.zip --no-check-certificate
  if not exist mysql-connector-c-6.1.6-win32.zip goto end_mysqlconnector_setup
    unzip mysql-connector-c-6.1.6-win32.zip
    rm mysql-connector-c-6.1.6-win32.zip
:end_mysqlconnector_setup

if exist mysql++-3.1.0 goto end_mysqlpp_setup
  wget http://tangentsoft.net/mysql++/releases/mysql++-3.1.0.tar.gz --no-check-certificate
  if not exist mysql++-3.1.0.tar.gz goto end_mysqlpp_setup
    gzip -d -c mysql++-3.1.0.tar.gz | tar -x
    cd mysql++-3.1.0/lib
    sed "90d" common.h > common.h.bak
    mv common.h.bak common.h
    cd ..
    cd vc2008
    cat mysql++_mysqlpp.vcproj | sed "s/ConfigurationType=\"2\"/ConfigurationType=\"4\"/" | sed "s/_USRDLL;//" | sed "s/DLL_EXPORTS//" | sed "s/MYSQLPP_MAKING_DLL/MYSQLPP_NO_DLL/" > mysql++_mysqlpp.vcproj.new
    mv mysql++_mysqlpp.vcproj.new mysql++_mysqlpp.vcproj
    devenv mysql++.sln /upgrade
    SET CL=/I..\..\mysql-connector-c-6.1.6-win32\include
    devenv mysql++_mysqlpp.vcxproj /useenv /Build Debug
    devenv mysql++_mysqlpp.vcxproj /useenv /Build Release
    SET CL=
    cd ..
    mkdir include
    cd include
    mkdir mysql++
    cp ../lib/*.h mysql++
    cd ../../
    rm mysql++-3.1.0.tar.gz
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
    cmake ..
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

if exist boost_1_63_0 goto end_boost_setup
  wget --no-check-certificate https://sourceforge.net/projects/boost/files/boost/1.63.0/boost_1_63_0.zip/download -O boost_1_63_0.zip
  if not exist boost_1_63_0.zip goto end_boost_setup
    unzip boost_1_63_0.zip
    cd boost_1_63_0
    CALL bootstrap.bat
    if "%NUMBER_OF_PROCESSORS%" == "" (
      SET BJAM_PROCESSORS=
    ) else (
      SET BJAM_PROCESSORS="-j"%NUMBER_OF_PROCESSORS%
    )
    b2 %BJAM_PROCESSORS% --toolset=msvc-14.0 --build-type=complete --stagedir=stage link=static,shared runtime-link=shared stage
    b2 %BJAM_PROCESSORS% --toolset=msvc-14.0 --build-type=complete --with-python --stagedir=stage link=static,shared runtime-link=shared stage
    cd ..
    rm boost_1_63_0.zip
:end_boost_setup

if exist lua-5.3.1 goto end_lua_setup
  wget --no-check-certificate http://www.lua.org/ftp/lua-5.3.1.tar.gz
  if not exist lua-5.3.1.tar.gz goto end_lua_setup
    gzip -d -c lua-5.3.1.tar.gz | tar -x
    cd lua-5.3.1/src
    cp %~dp0/lua_cmakelists.txt CMakeLists.txt
    cmake .
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
