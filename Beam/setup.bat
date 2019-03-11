@ECHO OFF
SETLOCAL
SET ROOT="%cd%"
IF NOT EXIST Catch2-2.2.1 (
  git clone --branch v2.2.1 https://github.com/catchorg/Catch2.git Catch2-2.2.1
)
IF NOT EXIST cppunit-1.14.0 (
  wget http://dev-www.libreoffice.org/src/cppunit-1.14.0.tar.gz --no-check-certificate
  IF EXIST cppunit-1.14.0.tar.gz (
    gzip -d -c cppunit-1.14.0.tar.gz | tar -xf -
    PUSHD cppunit-1.14.0\src\cppunit
    msbuild cppunit.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=Debug
    msbuild cppunit.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=Release
    POPD
    rm cppunit-1.14.0.tar.gz
  )
)
IF NOT EXIST cryptopp610 (
  wget http://www.cryptopp.com/cryptopp610.zip --no-check-certificate
  IF EXIST cryptopp610.zip (
    mkdir cryptopp610
    PUSHD cryptopp610
    unzip ..\cryptopp610.zip
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
    PUSHD include
    mkdir cryptopp
    cp ..\*.h cryptopp
    POPD
    POPD
    rm cryptopp610.zip
  )
)
IF NOT EXIST zlib-1.2.11 (
  git clone --branch v1.2.11 https://github.com/madler/zlib.git zlib-1.2.11
  if EXIST zlib-1.2.11 (
    PUSHD zlib-1.2.11\contrib\vstudio\vc14
    cat zlibstat.vcxproj | sed "s/ZLIB_WINAPI;//" | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > zlibstat.vcxproj.new
    mv zlibstat.vcxproj.new zlibstat.vcxproj
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=Debug
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=ReleaseWithoutAsm
    POPD
  )
)
IF NOT EXIST mariadb-connector-c-3.0.6-src (
  wget https://downloads.mariadb.org/f/connector-c-3.0.6/mariadb-connector-c-3.0.6-src.zip -O mariadb-connector-c-3.0.6-src.zip --no-check-certificate
  IF EXIST mariadb-connector-c-3.0.6-src.zip (
    unzip mariadb-connector-c-3.0.6-src.zip
    PUSHD mariadb-connector-c-3.0.6-src
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./mariadb .
    PUSHD libmariadb
    cat mariadbclient.vcxproj | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > mariadbclient.vcxproj.new
    mv mariadbclient.vcxproj.new mariadbclient.vcxproj
    POPD
    cmake --build . --target mariadbclient --config Debug
    cmake --build . --target mariadbclient --config Release
    PUSHD include
    printf "#include ""mariadb_version.h""" > mysql_version.h
    echo. >> mysql_version.h
    printf "#include ""WinSock2.h""" >> mysql_version.h
    echo. >> mysql_version.h
    printf "#define CLIENT_LONG_PASSWORD 1" >> mysql_version.h
    echo. >> mysql_version.h
    POPD
    POPD
    rm -rf mariadb-connector-c-3.0.6-src.zip
  )
)
IF NOT EXIST mysql++-3.2.3 (
  git clone https://github.com/eidolonsystems/mysqlpp mysql++-3.2.3
  IF EXIST mysql++-3.2.3 (
    PUSHD mysql++-3.2.3\vc2005
    SET CL=/I"%ROOT%\mariadb-connector-c-3.0.6-src\include"
    devenv mysql++_mysqlpp.vcxproj /useenv /Build Debug
    devenv mysql++_mysqlpp.vcxproj /useenv /Build Release
    SET CL=
    POPD
  )
)
IF NOT EXIST yaml-cpp (
  git clone https://github.com/jbeder/yaml-cpp.git yaml-cpp
  IF EXIST yaml-cpp (
    PUSHD yaml-cpp
    git checkout 0f9a586ca1dc29c2ecb8dd715a315b93e3f40f79
    mkdir build
    PUSHD build
    cmake -G "Visual Studio 15 2017" ..
    cmake --build . --target ALL_BUILD --config Debug
    cmake --build . --target ALL_BUILD --config Release
    POPD
    POPD
  )
)
IF NOT EXIST tclap-1.2.1 (
  wget "http://downloads.sourceforge.net/project/tclap/tclap-1.2.1.tar.gz?r=&ts=1309913922&use_mirror=superb-sea2" -O tclap-1.2.1.tar.gz --no-check-certificate
  IF EXIST tclap-1.2.1.tar.gz (
    gzip -d -c tclap-1.2.1.tar.gz | tar -xf -
    rm tclap-1.2.1.tar.gz
  )
)
IF NOT EXIST openssl-1.0.2g (
  wget ftp://ftp.openssl.org/source/old/1.0.2/openssl-1.0.2g.tar.gz --no-check-certificate
  IF EXIST openssl-1.0.2g.tar.gz (
    gzip -d -c openssl-1.0.2g.tar.gz | tar -xf -
    PUSHD openssl-1.0.2g
    perl Configure VC-WIN32 no-asm --prefix="%ROOT%\openssl-1.0.2g"
    CALL .\ms\do_ms
    nmake -f .\ms\nt.mak
    POPD
    rm openssl-1.0.2g.tar.gz
  )
)
IF "%NUMBER_OF_PROCESSORS%" == "" (
  SET BJAM_PROCESSORS=
) else (
  SET BJAM_PROCESSORS="-j%NUMBER_OF_PROCESSORS%"
)
IF NOT EXIST boost_1_67_0 (
  wget https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.zip -O boost_1_67_0.zip --no-check-certificate
  IF EXIST boost_1_67_0.zip (
    unzip boost_1_67_0.zip
    PUSHD boost_1_67_0
    wget https://www.boost.org/patches/1_67_0/0003-Python-Fix-auto-linking-logic-Windows-only.patch --no-check-certificate
    git apply 0003-Python-Fix-auto-linking-logic-Windows-only.patch
    CALL bootstrap.bat vc141
    b2 %BJAM_PROCESSORS% --without-context --prefix="%ROOT%\boost_1_67_0" --build-type=complete toolset=msvc-14.1 link=static,shared runtime-link=shared install
    b2 %BJAM_PROCESSORS% --with-context --prefix="%ROOT%\boost_1_67_0" --build-type=complete toolset=msvc-14.1 link=static runtime-link=shared install
    POPD
    rm boost_1_67_0.zip
  )
)
IF NOT EXIST lua-5.3.1 (
  wget http://www.lua.org/ftp/lua-5.3.1.tar.gz --no-check-certificate
  IF EXIST lua-5.3.1.tar.gz (
    gzip -d -c lua-5.3.1.tar.gz | tar -xf -
    PUSHD lua-5.3.1\src
    cp %~dp0\CMakeFiles\lua.cmake CMakeLists.txt
    cmake -G "Visual Studio 15 2017" .
    cmake --build . --target ALL_BUILD --config Debug
    cmake --build . --target ALL_BUILD --config Release
    POPD
    rm lua-5.3.1.tar.gz
  )
)
IF NOT EXIST sqlite-amalgamation-3230100 (
  wget https://www.sqlite.org/2018/sqlite-amalgamation-3230100.zip --no-check-certificate
  IF EXIST sqlite-amalgamation-3230100.zip (
    unzip sqlite-amalgamation-3230100
    PUSHD sqlite-amalgamation-3230100
    cl /c /Zi /MDd /DSQLITE_USE_URI=1 sqlite3.c
    lib sqlite3.obj
    cp sqlite3.lib sqlite3d.lib
    rm sqlite3.obj
    cl /c /O2 /MD /DSQLITE_USE_URI=1 sqlite3.c
    lib sqlite3.obj
    POPD
    rm sqlite-amalgamation-3230100.zip
  )
)
IF NOT EXIST viper (
  git clone https://www.github.com/eidolonsystems/viper
)
IF EXIST viper (
  SET viper_commit="0631eff5a0a36d77bc45da1b0118dd49ea22953b"
  PUSHD viper
  FOR /f "usebackq tokens=*" %%a IN (`git log -1 ^| head -1 ^| awk "{ print $2 }"`) DO SET commit=%%a
  IF NOT "%commit%" == "%viper_commit%" (
    git checkout master
    git pull
    git checkout %viper_commit%
  )
  cmake -G "Visual Studio 15 2017" .
  POPD
)
pip install Sphinx sphinx-jsondomain sphinx_rtd_theme sphinxcontrib-httpdomain --user
ENDLOCAL
