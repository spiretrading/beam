@ECHO OFF
SETLOCAL
SET ROOT="%cd%"
SET VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /f "usebackq delims=" %%i IN (`%VSWHERE% -prerelease -latest -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat"
  )
)
IF NOT EXIST Strawberry (
  wget http://strawberryperl.com/download/5.28.2.1/strawberry-perl-5.28.2.1-64bit-portable.zip -O strawberry-perl-5.28.2.1-64bit-portable.zip --no-check-certificate
  MD Strawberry
  PUSHD Strawberry
  unzip ..\strawberry-perl-5.28.2.1-64bit-portable.zip
  POPD
  DEL strawberry-perl-5.28.2.1-64bit-portable.zip
)
SET PATH=%PATH%;%ROOT%\Strawberry\perl\site\bin;%ROOT%\Strawberry\perl\bin;%ROOT%\Strawberry\c\bin
IF NOT EXIST cppunit-1.14.0 (
  wget http://dev-www.libreoffice.org/src/cppunit-1.14.0.tar.gz --no-check-certificate
  IF EXIST cppunit-1.14.0.tar.gz (
    gzip -d -c cppunit-1.14.0.tar.gz | tar -xf -
    PUSHD cppunit-1.14.0\src\cppunit
    msbuild cppunit.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=Debug
    msbuild cppunit.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=Release
    POPD
    DEL cppunit-1.14.0.tar.gz
  )
)
IF NOT EXIST cryptopp610 (
  wget http://www.cryptopp.com/cryptopp610.zip --no-check-certificate
  IF EXIST cryptopp610.zip (
    MD cryptopp610
    PUSHD cryptopp610
    unzip ..\cryptopp610.zip
    devenv /Upgrade cryptlib.vcproj
    TYPE cryptlib.vcproj | sed "s/WholeProgramOptimization=\"1\"/WholeProgramOptimization=\"0\"/" | sed "s/WholeProgramOptimization=\"true\"/WholeProgramOptimization=\"false\"/" > cryptlib.vcproj.new
    MOVE cryptlib.vcproj.new cryptlib.vcproj
    TYPE cryptlib.vcxproj | sed "s/<WholeProgramOptimization>true<\/WholeProgramOptimization>/<WholeProgramOptimization>false<\/WholeProgramOptimization>/" > cryptlib.vcxproj.new
    MOVE cryptlib.vcxproj.new cryptlib.vcxproj
    TYPE cryptlib.vcproj | sed "s/RuntimeLibrary=\"0\"/RuntimeLibrary=\"2\"/" | sed "s/RuntimeLibrary=\"1\"/RuntimeLibrary=\"3\"/" > cryptlib.vcproj.new
    MOVE cryptlib.vcproj.new cryptlib.vcproj
    TYPE cryptlib.vcxproj | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > cryptlib.vcxproj.new
    MOVE cryptlib.vcxproj.new cryptlib.vcxproj
    devenv cryptlib.vcxproj /useenv /Build "Debug"
    devenv cryptlib.vcxproj /useenv /Build "Release"
    MD include
    PUSHD include
    MD cryptopp
    COPY ..\*.h cryptopp
    POPD
    POPD
    DEL cryptopp610.zip
  )
)
IF NOT EXIST lua-5.3.1 (
  wget http://www.lua.org/ftp/lua-5.3.1.tar.gz --no-check-certificate
  IF EXIST lua-5.3.1.tar.gz (
    gzip -d -c lua-5.3.1.tar.gz | tar -xf -
    PUSHD lua-5.3.1\src
    COPY %~dp0\Config\lua.cmake CMakeLists.txt
    cmake .
    cmake --build . --target ALL_BUILD --config Debug
    cmake --build . --target ALL_BUILD --config Release
    POPD
    DEL lua-5.3.1.tar.gz
  )
)
IF NOT EXIST mariadb-connector-c-3.0.6-src (
  wget https://downloads.mariadb.org/f/connector-c-3.0.6/mariadb-connector-c-3.0.6-src.zip -O mariadb-connector-c-3.0.6-src.zip --no-check-certificate
  IF EXIST mariadb-connector-c-3.0.6-src.zip (
    unzip mariadb-connector-c-3.0.6-src.zip
    PUSHD mariadb-connector-c-3.0.6-src
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./mariadb .
    PUSHD libmariadb
    TYPE mariadbclient.vcxproj | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > mariadbclient.vcxproj.new
    MOVE mariadbclient.vcxproj.new mariadbclient.vcxproj
    POPD
    cmake --build . --target mariadbclient --config Debug
    cmake --build . --target mariadbclient --config Release
    PUSHD include
    printf "#include ""mariadb_version.h""" > mysql_version.h
    ECHO. >> mysql_version.h
    printf "#include ""WinSock2.h""" >> mysql_version.h
    ECHO. >> mysql_version.h
    printf "#define CLIENT_LONG_PASSWORD 1" >> mysql_version.h
    ECHO. >> mysql_version.h
    POPD
    POPD
    DEL mariadb-connector-c-3.0.6-src.zip
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
    DEL openssl-1.0.2g.tar.gz
  )
)
IF NOT EXIST Python-3.7.2 (
  wget https://www.python.org/ftp/python/3.7.2/Python-3.7.2.tgz --no-check-certificate
  IF EXIST Python-3.7.2.tgz (
    gzip -d -c Python-3.7.2.tgz | tar -xf -
    PUSHD Python-3.7.2
    PUSHD PCbuild
    CALL build.bat -E -c Debug -p Win32
    CALL build.bat -E -c Release -p Win32
    POPD
    COPY PC\pyconfig.h Include
    POPD
    DEL Python-3.7.2.tgz
  )
)
IF NOT EXIST sqlite-amalgamation-3230100 (
  wget https://www.sqlite.org/2018/sqlite-amalgamation-3230100.zip --no-check-certificate
  IF EXIST sqlite-amalgamation-3230100.zip (
    unzip sqlite-amalgamation-3230100
    PUSHD sqlite-amalgamation-3230100
    cl /c /Zi /MDd /DSQLITE_USE_URI=1 sqlite3.c
    lib sqlite3.obj
    COPY sqlite3.lib sqlite3d.lib
    DEL sqlite3.obj
    cl /c /O2 /MD /DSQLITE_USE_URI=1 sqlite3.c
    lib sqlite3.obj
    POPD
    DEL sqlite-amalgamation-3230100.zip
  )
)
IF NOT EXIST tclap-1.2.1 (
  wget "http://downloads.sourceforge.net/project/tclap/tclap-1.2.1.tar.gz?r=&ts=1309913922&use_mirror=superb-sea2" -O tclap-1.2.1.tar.gz --no-check-certificate
  IF EXIST tclap-1.2.1.tar.gz (
    gzip -d -c tclap-1.2.1.tar.gz | tar -xf -
    DEL tclap-1.2.1.tar.gz
  )
)
SET BUILD_VIPER=
IF NOT EXIST viper (
  git clone https://www.github.com/eidolonsystems/viper
  SET BUILD_VIPER=1
)
SET viper_commit="eb1894162f8a809c60cbc3a5f640a591b67e4863"
PUSHD viper
git merge-base --is-ancestor "%viper_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%viper_commit%"
  SET BUILD_VIPER=1
)
IF "%BUILD_VIPER%" == "1" (
  CALL run_cmake.bat "-DD=%ROOT%"
  CALL build.bat Debug
  CALL build.bat Release
)
POPD
SET commit=
IF NOT EXIST yaml-cpp (
  git clone https://github.com/jbeder/yaml-cpp.git yaml-cpp
  IF EXIST yaml-cpp (
    PUSHD yaml-cpp
    git checkout 0f9a586ca1dc29c2ecb8dd715a315b93e3f40f79
    MD build
    PUSHD build
    cmake ..
    cmake --build . --target ALL_BUILD --config Debug
    cmake --build . --target ALL_BUILD --config Release
    POPD
    POPD
  )
)
IF NOT EXIST zlib-1.2.11 (
  git clone --branch v1.2.11 https://github.com/madler/zlib.git zlib-1.2.11
  IF EXIST zlib-1.2.11 (
    PUSHD zlib-1.2.11\contrib\vstudio\vc14
    TYPE zlibstat.vcxproj | sed "s/ZLIB_WINAPI;//" | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > zlibstat.vcxproj.new
    MOVE zlibstat.vcxproj.new zlibstat.vcxproj
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=Debug
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v141 /p:Configuration=ReleaseWithoutAsm
    POPD
  )
)
IF "%NUMBER_OF_PROCESSORS%" == "" (
  SET BJAM_PROCESSORS=
) ELSE (
  SET BJAM_PROCESSORS="-j%NUMBER_OF_PROCESSORS%"
)
SET BJAM_ROOT="%ROOT:\=/%"
SET BJAM_ROOT="%BJAM_ROOT::=\:%"
SET BJAM_CONFIG=using python : 3.7 : %BJAM_ROOT%/Python-3.7.2 : %BJAM_ROOT%/Python-3.7.2/Include : %BJAM_ROOT%/Python-3.7.2/PCBuild/win32 ;
IF NOT EXIST boost_1_70_0 (
  wget https://dl.bintray.com/boostorg/release/1.70.0/source/boost_1_70_0.zip -O boost_1_70_0.zip --no-check-certificate
  IF EXIST boost_1_70_0.zip (
    unzip boost_1_70_0.zip
    PUSHD boost_1_70_0
    ECHO %BJAM_CONFIG% > tools\build\src\user-config.jam
    CALL bootstrap.bat vc141
    b2 %BJAM_PROCESSORS% --without-context --prefix="%ROOT%\boost_1_70_0" --build-type=complete address-model=32 toolset=msvc-14.1 link=static,shared runtime-link=shared install
    b2 %BJAM_PROCESSORS% --with-context --prefix="%ROOT%\boost_1_70_0" --build-type=complete address-model=32 toolset=msvc-14.1 link=static runtime-link=shared install
    POPD
    DEL boost_1_70_0.zip
  )
)
ENDLOCAL
