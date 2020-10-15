@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET EXIT_STATUS=0
SET ROOT=%cd%
IF EXIST cache_files\beam.txt (
  FOR /F %%i IN (
      'ls -l --time-style=full-iso "%~dp0\setup.bat" ^| awk "{print $6 $7}"') DO (
    FOR /F %%j IN (
        'ls -l --time-style=full-iso cache_files\beam.txt ^| awk "{print $6 $7}"') DO (
      IF "%%i" LSS "%%j" (
        EXIT /B 0
      )
    )
  )
)
SET VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /f "usebackq delims=" %%i IN (`!VSWHERE! -prerelease -latest -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat"
  )
)
IF NOT EXIST Strawberry (
  wget http://strawberryperl.com/download/5.30.1.1/strawberry-perl-5.30.1.1-64bit-portable.zip -O strawberry-perl-5.30.1.1-64bit-portable.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    MD Strawberry
    PUSHD Strawberry
    unzip ..\strawberry-perl-5.30.1.1-64bit-portable.zip
    POPD
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q strawberry-perl-5.30.1.1-64bit-portable.zip
)
SET PATH=!PATH!;!ROOT!\Strawberry\perl\site\bin;!ROOT!\Strawberry\perl\bin;!ROOT!\Strawberry\c\bin
SET BUILD_ASPEN=
SET ASPEN_COMMIT="1d8d6288b130e00c398c5c82af621f20ebbbff3f"
IF NOT EXIST aspen (
  git clone https://www.github.com/spiretrading/aspen
  IF !ERRORLEVEL! EQU 0 (
    SET BUILD_ASPEN=1
    PUSHD aspen
    git checkout "!ASPEN_COMMIT!"
    POPD
  ) ELSE (
    RD /S /Q aspen
    SET EXIT_STATUS=1
  )
)
IF EXIST aspen (
  PUSHD aspen
  git merge-base --is-ancestor "!ASPEN_COMMIT!" HEAD
  IF !ERRORLEVEL! NEQ 0 (
    git checkout master
    git pull
    git checkout "!ASPEN_COMMIT!"
    SET BUILD_ASPEN=1
  )
  IF !BUILD_ASPEN! EQU 1 (
    CALL configure.bat -DD="!ROOT!"
    CALL build.bat Debug
    CALL build.bat Release
  ) ELSE (
    PUSHD !ROOT!
    CALL aspen\setup.bat
    POPD
  )
  POPD
)
IF NOT EXIST cryptopp820 (
  wget https://www.cryptopp.com/cryptopp820.zip -O cryptopp820.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
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
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q cryptopp820.zip
)
IF NOT EXIST doctest-2.3.6 (
  wget https://github.com/onqtam/doctest/archive/2.3.6.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    unzip 2.3.6.zip
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q 2.3.6.zip
)
IF NOT EXIST mariadb-connector-c-3.1.7 (
  wget https://github.com/MariaDB/mariadb-connector-c/archive/v3.1.7.zip -O mariadb-connector-c-3.1.7.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    unzip mariadb-connector-c-3.1.7.zip
    PUSHD mariadb-connector-c-3.1.7
    cmake -A Win32 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./mariadb .
    PUSHD libmariadb
    TYPE mariadbclient.vcxproj | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > mariadbclient.vcxproj.new
    MOVE mariadbclient.vcxproj.new mariadbclient.vcxproj
    POPD
    cmake --build . --target mariadbclient --config Debug
    cmake --build . --target mariadbclient --config Release
    POPD
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q mariadb-connector-c-3.1.7.zip
)
IF NOT EXIST openssl-1.1.1c (
  wget https://ftp.openssl.org/source/old/1.1.1/openssl-1.1.1c.tar.gz -O openssl-1.1.1c.tar.gz --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    gzip -d -c openssl-1.1.1c.tar.gz | tar -xf -
    MOVE openssl-1.1.1c openssl-1.1.1c-build
    PUSHD openssl-1.1.1c-build
    perl Configure VC-WIN32 no-asm no-shared no-tests --prefix="!ROOT!\openssl-1.1.1c" --openssldir="!ROOT!\openssl-1.1.1c"
    SET CL=/MP
    nmake
    nmake install
    POPD
    RD /S /Q openssl-1.1.1c-build
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q openssl-1.1.1c.tar.gz
)
IF NOT EXIST sqlite-amalgamation-3300100 (
  wget https://www.sqlite.org/2019/sqlite-amalgamation-3300100.zip -O sqlite-amalgamation-3300100.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    unzip sqlite-amalgamation-3300100.zip
    PUSHD sqlite-amalgamation-3300100
    cl /c /Zi /MDd /DSQLITE_USE_URI=1 sqlite3.c
    lib sqlite3.obj
    COPY sqlite3.lib sqlite3d.lib
    DEL sqlite3.obj
    cl /c /O2 /MD /DSQLITE_USE_URI=1 sqlite3.c
    lib sqlite3.obj
    POPD
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q sqlite-amalgamation-3300100.zip
)
IF NOT EXIST tclap-1.2.2 (
  wget https://github.com/mirror/tclap/archive/v1.2.2.zip -O v1.2.2.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    unzip v1.2.2.zip
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q v1.2.2.zip
)
SET VIPER_COMMIT="99212c81d5a4b5b60bf9a585cdf51eec62888bc7"
IF NOT EXIST viper (
  git clone https://www.github.com/spiretrading/viper
  IF !ERRORLEVEL! EQU 0 (
    PUSHD viper
    git checkout "!VIPER_COMMIT!"
    POPD
  ) ELSE (
    RD /S /Q viper
    SET EXIT_STATUS=1
  )
)
IF EXIST viper (
  PUSHD viper
  git merge-base --is-ancestor "!VIPER_COMMIT!" HEAD
  IF !ERRORLEVEL! NEQ 0 (
    git checkout master
    git pull
    git checkout "!VIPER_COMMIT!"
  )
  POPD
)
IF NOT EXIST yaml-cpp (
  git clone https://github.com/jbeder/yaml-cpp.git yaml-cpp
  IF !ERRORLEVEL! EQU 0 (
    PUSHD yaml-cpp
    git checkout 0f9a586ca1dc29c2ecb8dd715a315b93e3f40f79
    MD build
    PUSHD build
    cmake -A Win32 ..
    cmake --build . --target ALL_BUILD --config Debug
    cmake --build . --target ALL_BUILD --config Release
    POPD
    POPD
  ) ELSE (
    RD /S /Q yaml-cpp
    SET EXIT_STATUS=1
  )
)
IF NOT EXIST zlib-1.2.11 (
  git clone --branch v1.2.11 https://github.com/madler/zlib.git zlib-1.2.11
  IF !ERRORLEVEL! EQU 0 (
    PUSHD zlib-1.2.11\contrib\vstudio\vc14
    TYPE zlibstat.vcxproj | sed "s/ZLIB_WINAPI;//" | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > zlibstat.vcxproj.new
    MOVE zlibstat.vcxproj.new zlibstat.vcxproj
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v142 /p:Configuration=Debug
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v142 /p:Configuration=ReleaseWithoutAsm
    POPD
  ) ELSE (
    RD /S /Q zlib-1.2.11
    SET EXIT_STATUS=1
  )
)
IF "%NUMBER_OF_PROCESSORS%" == "" (
  SET BJAM_PROCESSORS=
) ELSE (
  SET BJAM_PROCESSORS="-j%NUMBER_OF_PROCESSORS%"
)
IF NOT EXIST boost_1_72_0 (
  wget https://dl.bintray.com/boostorg/release/1.72.0/source/boost_1_72_0.zip -O boost_1_72_0.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    unzip boost_1_72_0.zip
    PUSHD boost_1_72_0
    PUSHD tools\build
    CALL bootstrap.bat vc142
    POPD
    tools\build\b2 !BJAM_PROCESSORS! --without-context --prefix="!ROOT!\boost_1_72_0" --build-type=complete address-model=32 toolset=msvc-14.2 link=static,shared runtime-link=shared install
    tools\build\b2 !BJAM_PROCESSORS! --with-context --prefix="!ROOT!\boost_1_72_0" --build-type=complete address-model=32 toolset=msvc-14.2 link=static runtime-link=shared install
    POPD
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q boost_1_72_0.zip
)
IF NOT EXIST cache_files (
  MD cache_files
)
ECHO timestamp > cache_files\beam.txt
ENDLOCAL
EXIT /B !EXIT_STATUS!
