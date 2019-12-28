@ECHO OFF
SETLOCAL
SET ROOT=%cd%
SET VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /f "usebackq delims=" %%i IN (`%VSWHERE% -prerelease -latest -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat"
  )
)
IF NOT EXIST Strawberry (
  wget http://strawberryperl.com/download/5.30.1.1/strawberry-perl-5.30.1.1-64bit-portable.zip -O strawberry-perl-5.30.1.1-64bit-portable.zip --no-check-certificate
  MD Strawberry
  PUSHD Strawberry
  unzip ..\strawberry-perl-5.30.1.1-64bit-portable.zip
  POPD
  DEL strawberry-perl-5.30.1.1-64bit-portable.zip
)
SET PATH=%PATH%;%ROOT%\Strawberry\perl\site\bin;%ROOT%\Strawberry\perl\bin;%ROOT%\Strawberry\c\bin
IF NOT EXIST cppunit-1.14.0 (
  wget https://github.com/freedesktop/libreoffice-cppunit/archive/cppunit-1.14.0.zip -O cppunit-1.14.0.zip --no-check-certificate
  IF EXIST cppunit-1.14.0.zip (
    unzip cppunit-1.14.0.zip
    MV libreoffice-cppunit-cppunit-1.14.0 cppunit-1.14.0
    PUSHD cppunit-1.14.0\src\cppunit
    msbuild cppunit.vcxproj /p:UseEnv=True /p:PlatformToolset=v142 /p:Configuration=Debug
    msbuild cppunit.vcxproj /p:UseEnv=True /p:PlatformToolset=v142 /p:Configuration=Release
    POPD
    DEL cppunit-1.14.0.zip
  )
)
SET BUILD_ASPEN=
IF NOT EXIST aspen (
  git clone https://www.github.com/eidolonsystems/aspen
  SET BUILD_ASPEN=1
)
SET aspen_commit="986385c8fa17270165496c3bcd8c454da0ad9580"
PUSHD aspen
git merge-base --is-ancestor "%aspen_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%aspen_commit%"
  SET BUILD_ASPEN=1
)
IF "%BUILD_ASPEN%" == "1" (
  CALL configure.bat -DD="%ROOT%"
  CALL build.bat Debug
  CALL build.bat Release
) ELSE (
  PUSHD %ROOT%
  CALL aspen\setup.bat
  POPD
)
POPD
IF NOT EXIST cryptopp820 (
  wget https://www.cryptopp.com/cryptopp820.zip -O cryptopp820.zip --no-check-certificate
  IF EXIST cryptopp820.zip (
    MD cryptopp820
    PUSHD cryptopp820
    unzip ..\cryptopp820.zip
    TYPE cryptlib.vcxproj | sed "s/<WholeProgramOptimization>true<\/WholeProgramOptimization>/<WholeProgramOptimization>false<\/WholeProgramOptimization>/" > cryptlib.vcxproj.new
    MOVE cryptlib.vcxproj.new cryptlib.vcxproj
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
    DEL cryptopp820.zip
  )
)
IF NOT EXIST doctest-2.3.6 (
  wget https://github.com/onqtam/doctest/archive/2.3.6.zip --no-check-certificate
  IF EXIST 2.3.6.zip (
    unzip 2.3.6.zip
    DEL 2.3.6.zip
  )
)
IF NOT EXIST mariadb-connector-c-3.0.6 (
  wget https://github.com/MariaDB/mariadb-connector-c/archive/v3.0.6.zip -O mariadb-connector-c-3.0.6.zip --no-check-certificate
  IF EXIST mariadb-connector-c-3.0.6.zip (
    unzip mariadb-connector-c-3.0.6.zip
    PUSHD mariadb-connector-c-3.0.6
    cmake -A Win32 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./mariadb .
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
    DEL mariadb-connector-c-3.0.6.zip
  )
)
IF NOT EXIST openssl-1.1.1c (
  wget https://ftp.openssl.org/source/old/1.1.1/openssl-1.1.1c.tar.gz -O openssl-1.1.1c.tar.gz --no-check-certificate
  IF EXIST openssl-1.1.1c.tar.gz (
    gzip -d -c openssl-1.1.1c.tar.gz | tar -xf -
    MOVE openssl-1.1.1c openssl-1.1.1c-build
    PUSHD openssl-1.1.1c-build
    perl Configure VC-WIN32 no-asm no-shared no-tests --prefix="%ROOT%\openssl-1.1.1c" --openssldir="%ROOT%\openssl-1.1.1c"
    SET CL=/MP
    nmake
    nmake install
    POPD
    RMDIR /S /Q openssl-1.1.1c-build
    DEL openssl-1.1.1c.tar.gz
  )
)
IF NOT EXIST sqlite-amalgamation-3300100 (
  wget https://www.sqlite.org/2019/sqlite-amalgamation-3300100.zip -O sqlite-amalgamation-3300100.zip --no-check-certificate
  IF EXIST sqlite-amalgamation-3300100.zip (
    unzip sqlite-amalgamation-3300100.zip
    PUSHD sqlite-amalgamation-3300100
    cl /c /Zi /MDd /DSQLITE_USE_URI=1 sqlite3.c
    lib sqlite3.obj
    COPY sqlite3.lib sqlite3d.lib
    DEL sqlite3.obj
    cl /c /O2 /MD /DSQLITE_USE_URI=1 sqlite3.c
    lib sqlite3.obj
    POPD
    DEL sqlite-amalgamation-3300100.zip
  )
)
IF NOT EXIST tclap-1.2.2 (
  wget https://github.com/mirror/tclap/archive/v1.2.2.zip -O v1.2.2.zip --no-check-certificate
  IF EXIST v1.2.2.zip (
    unzip v1.2.2.zip
    DEL v1.2.2.zip
  )
)
SET viper_commit="3998912cecaaa041b2dea37485905b3345797744"
IF NOT EXIST viper (
  git clone https://www.github.com/eidolonsystems/viper
)
PUSHD viper
git merge-base --is-ancestor "%viper_commit%" HEAD
IF NOT "%ERRORLEVEL%" == "0" (
  git checkout master
  git pull
  git checkout "%viper_commit%"
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
    cmake -A Win32 ..
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
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v142 /p:Configuration=Debug
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v142 /p:Configuration=ReleaseWithoutAsm
    POPD
  )
)
IF "%NUMBER_OF_PROCESSORS%" == "" (
  SET BJAM_PROCESSORS=
) ELSE (
  SET BJAM_PROCESSORS="-j%NUMBER_OF_PROCESSORS%"
)
IF NOT EXIST boost_1_72_0 (
  wget https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.zip -O boost_1_72_0.zip --no-check-certificate
  IF EXIST boost_1_72_0.zip (
    unzip boost_1_72_0.zip
    PUSHD boost_1_72_0
    PUSHD tools\build
    CALL bootstrap.bat vc142
    POPD
    tools\build\b2 %BJAM_PROCESSORS% --without-context --prefix="%ROOT%\boost_1_72_0" --build-type=complete address-model=32 toolset=msvc-14.2 link=static,shared runtime-link=shared install
    tools\build\b2 %BJAM_PROCESSORS% --with-context --prefix="%ROOT%\boost_1_72_0" --build-type=complete address-model=32 toolset=msvc-14.2 link=static runtime-link=shared install
    POPD
    DEL boost_1_72_0.zip
  )
)
ENDLOCAL
