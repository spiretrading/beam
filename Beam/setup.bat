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
  wget https://strawberryperl.com/download/5.32.1.1/strawberry-perl-5.32.1.1-64bit-portable.zip -O strawberry-perl-5.32.1.1-64bit-portable.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    MD Strawberry
    PUSHD Strawberry
    tar -xf ..\strawberry-perl-5.32.1.1-64bit-portable.zip
    POPD
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q strawberry-perl-5.32.1.1-64bit-portable.zip
)
SET PATH=!PATH!;!ROOT!\Strawberry\perl\site\bin;!ROOT!\Strawberry\perl\bin;!ROOT!\Strawberry\c\bin
SET BUILD_ASPEN=
SET ASPEN_COMMIT="baa1acb9ea7a7dd780b932929e49591ac358066e"
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
IF NOT EXIST cryptopp870 (
  wget https://github.com/weidai11/cryptopp/archive/refs/tags/CRYPTOPP_8_7_0.zip -O cryptopp870.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    tar -xf cryptopp870.zip
    MOVE cryptopp-CRYPTOPP_8_7_0 cryptopp870
    PUSHD cryptopp870
    TYPE cryptlib.vcxproj | sed "s/<WholeProgramOptimization>true<\/WholeProgramOptimization>/<WholeProgramOptimization>false<\/WholeProgramOptimization>/" > cryptlib.vcxproj.new
    MOVE cryptlib.vcxproj.new cryptlib.vcxproj
    TYPE cryptlib.vcxproj | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > cryptlib.vcxproj.new
    MOVE cryptlib.vcxproj.new cryptlib.vcxproj
    msbuild /t:Build /p:UseEnv=True /p:PlatformToolset=v143 /p:Configuration=Debug;Platform=Win32 cryptlib.vcxproj
    msbuild /t:Build /p:UseEnv=True /p:PlatformToolset=v143 /p:Configuration=Release;Platform=Win32 cryptlib.vcxproj
    MD include
    PUSHD include
    MD cryptopp
    COPY ..\*.h cryptopp
    POPD
    POPD
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q cryptopp870.zip
)
IF NOT EXIST mariadb-connector-c-3.3.3 (
  wget https://github.com/mariadb-corporation/mariadb-connector-c/archive/refs/tags/v3.3.3.zip -O mariadb-connector-c-3.3.3.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    tar -xf mariadb-connector-c-3.3.3.zip
    PUSHD mariadb-connector-c-3.3.3
    cmake -A Win32 -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./mariadb .
    PUSHD libmariadb
    TYPE mariadbclient.vcxproj | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > mariadbclient.vcxproj.new
    MOVE mariadbclient.vcxproj.new mariadbclient.vcxproj
    TYPE mariadb_obj.vcxproj | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > mariadb_obj.vcxproj.new
    MOVE mariadb_obj.vcxproj.new mariadb_obj.vcxproj
    POPD
    cmake --build . --target mariadbclient --config Debug
    cmake --build . --target mariadbclient --config Release
    POPD
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q mariadb-connector-c-3.3.3.zip
)
IF NOT EXIST openssl-1.1.1q (
  wget https://www.openssl.org/source/old/1.1.1/openssl-1.1.1q.tar.gz -O openssl-1.1.1q.tar.gz --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    gzip -d -c openssl-1.1.1q.tar.gz | tar -xf -
    MOVE openssl-1.1.1q openssl-1.1.1q-build
    PUSHD openssl-1.1.1q-build
    perl Configure VC-WIN32 no-asm no-shared no-tests --prefix="!ROOT!\openssl-1.1.1q" --openssldir="!ROOT!\openssl-1.1.1q"
    SET CL=/MP
    nmake
    nmake install
    POPD
    RD /S /Q openssl-1.1.1q-build
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q openssl-1.1.1q.tar.gz
)
IF NOT EXIST sqlite-amalgamation-3400100 (
  wget https://www.sqlite.org/2022/sqlite-amalgamation-3400100.zip -O sqlite-amalgamation-3400100.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    tar -xf sqlite-amalgamation-3400100.zip
    PUSHD sqlite-amalgamation-3400100
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
  DEL /F /Q sqlite-amalgamation-3400100.zip
)
IF NOT EXIST tclap-1.2.5 (
  wget https://github.com/mirror/tclap/archive/v1.2.5.zip -O v1.2.5.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    tar -xf v1.2.5.zip
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q v1.2.5.zip
)
SET VIPER_COMMIT="f0e05acaadb41abf3e1632b76b9c5f9fb5b8af99"
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
IF NOT EXIST zlib-1.2.13 (
  git clone --branch v1.2.13 https://github.com/madler/zlib.git zlib-1.2.13
  IF !ERRORLEVEL! EQU 0 (
    PUSHD zlib-1.2.13\contrib\vstudio\vc14
    TYPE zlibstat.vcxproj | sed "s/ZLIB_WINAPI;//" | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > zlibstat.vcxproj.new
    MOVE zlibstat.vcxproj.new zlibstat.vcxproj
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v143 /p:Configuration=Debug;Platform=Win32
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v143 /p:Configuration=ReleaseWithoutAsm;Platform=Win32
    POPD
  ) ELSE (
    RD /S /Q zlib-1.2.13
    SET EXIT_STATUS=1
  )
)
IF "%NUMBER_OF_PROCESSORS%" == "" (
  SET BJAM_PROCESSORS=
) ELSE (
  SET BJAM_PROCESSORS="-j%NUMBER_OF_PROCESSORS%"
)
IF NOT EXIST boost_1_86_0 (
  wget https://archives.boost.io/release/1.86.0/source/boost_1_86_0.zip -O boost_1_86_0.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    tar -xf boost_1_86_0.zip
    PUSHD boost_1_86_0
    PUSHD tools\build
    CALL bootstrap.bat vc143
    POPD
    tools\build\b2 !BJAM_PROCESSORS! --without-context --prefix="!ROOT!\boost_1_86_0" --build-type=complete address-model=32 toolset=msvc-14.3 link=static,shared runtime-link=shared install
    tools\build\b2 !BJAM_PROCESSORS! --with-context --prefix="!ROOT!\boost_1_86_0" --build-type=complete address-model=32 toolset=msvc-14.3 link=static runtime-link=shared install
    POPD
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q boost_1_86_0.zip
)
IF NOT EXIST cache_files (
  MD cache_files
)
ECHO timestamp > cache_files\beam.txt
ENDLOCAL
EXIT /B !EXIT_STATUS!
