@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET EXIT_STATUS=0
SET ROOT=%cd%
FOR /F "skip=1" %%H IN ('certutil -hashfile "%~dp0setup.bat" SHA256') DO (
  IF NOT DEFINED SETUP_HASH SET SETUP_HASH=%%H
)
IF EXIST cache_files\beam.txt (
  SET /P CACHED_HASH=<cache_files\beam.txt
  IF "!SETUP_HASH!"=="!CACHED_HASH!" EXIT /B 0
)
SET VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /F "usebackq delims=" %%i IN (` ^
    !VSWHERE! -prerelease -latest -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat" -arch=amd64
  )
)
CALL :DownloadAndExtract "Strawberry" ^
  "https://github.com/StrawberryPerl/Perl-Dist-Strawberry/releases/download/SP_54201_64bit/strawberry-perl-5.42.0.1-64bit-portable.zip" ^
  "a1cde185656cf307b51670eed69f648b9eff15b5c518cb136e027c628e650b71"
SET PATH=!PATH!;!ROOT!\Strawberry\perl\site\bin;!ROOT!\Strawberry\perl\bin;!ROOT!\Strawberry\c\bin
SET BUILD_ASPEN=
SET ASPEN_COMMIT="fa0573bf1df7cfb896c67bc09efbd1de0da94ba8"
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
CALL :DownloadAndExtract "cryptopp890" ^
  "https://github.com/weidai11/cryptopp/archive/refs/tags/CRYPTOPP_8_9_0.zip" ^
  "b885403cb13d490bebe90f25fad7150b88857f7acf3bd8b9ca1cec04c9ec8a51"
IF !BUILD_NEEDED!==1 (
  PUSHD cryptopp890
  powershell -Command "(Get-Content cryptlib.vcxproj) -replace " ^
    "'<WholeProgramOptimization>true</WholeProgramOptimization>', " ^
    "'<WholeProgramOptimization>false</WholeProgramOptimization>' | " ^
    "Set-Content cryptlib.vcxproj"
  powershell -Command "(Get-Content cryptlib.vcxproj) -replace " ^
    "'<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>', " ^
    "'<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>' -replace " ^
    "'<RuntimeLibrary>MultiThreaded</RuntimeLibrary>', " ^
    "'<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>' | " ^
    "Set-Content cryptlib.vcxproj"
  msbuild /t:Build /p:UseEnv=True /p:PlatformToolset=v143 /p:Platform=x64 ^
    /p:Configuration=Debug cryptlib.vcxproj
  msbuild /t:Build /p:UseEnv=True /p:PlatformToolset=v143 /p:Platform=x64 ^
    /p:Configuration=Release cryptlib.vcxproj
  MD include
  PUSHD include
  MD cryptopp
  COPY ..\*.h cryptopp
  POPD
  POPD
)
CALL :DownloadAndExtract "mariadb-connector-c-3.4.8" ^
  "https://github.com/mariadb-corporation/mariadb-connector-c/archive/refs/tags/v3.4.8.zip" ^
  "d8d91088ff33bbfe0d469f0ad50f472a39a2a1d9c9d975aa31c0dbbf504e425f"
IF !BUILD_NEEDED!==1 (
  PUSHD mariadb-connector-c-3.4.8
  cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./mariadb .
  PUSHD libmariadb
  powershell -Command "(Get-Content mariadbclient.vcxproj) -replace " ^
    "'<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>', " ^
    "'<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>' -replace " ^
    "'<RuntimeLibrary>MultiThreaded</RuntimeLibrary>', " ^
    "'<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>' | " ^
    "Set-Content mariadbclient.vcxproj"
  powershell -Command "(Get-Content mariadb_obj.vcxproj) -replace " ^
    "'<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>', " ^
    "'<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>' -replace " ^
    "'<RuntimeLibrary>MultiThreaded</RuntimeLibrary>', " ^
    "'<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>' | " ^
    "Set-Content mariadb_obj.vcxproj"
  POPD
  cmake --build . --target mariadbclient --config Debug
  cmake --build . --target mariadbclient --config Release
  POPD
)
CALL :DownloadAndExtract "openssl-3.6.0" ^
  "https://github.com/openssl/openssl/archive/refs/tags/openssl-3.6.0.zip" ^
  "273d989d1157f0bd494054e1b799b6bdba39d4acaff6dfcb8db02656f1b454dd"
IF !BUILD_NEEDED!==1 (
  MOVE openssl-3.6.0 openssl-3.6.0-build
  PUSHD openssl-3.6.0-build
  perl Configure VC-WIN64A no-asm no-shared no-tests ^
    --prefix="!ROOT!\openssl-3.6.0" --openssldir="!ROOT!\openssl-3.6.0"
  SET CL=/MP
  nmake
  nmake install
  POPD
  RD /S /Q openssl-3.6.0-build
)
CALL :DownloadAndExtract "sqlite-amalgamation-3510200" ^
  "https://www.sqlite.org/2026/sqlite-amalgamation-3510200.zip" ^
  "6e2a845a493026bdbad0618b2b5a0cf48584faab47384480ed9f592d912f23ec"
IF !BUILD_NEEDED!==1 (
  PUSHD sqlite-amalgamation-3510200
  cl /c /Zi /MDd /DSQLITE_USE_URI=1 sqlite3.c
  lib sqlite3.obj
  COPY sqlite3.lib sqlite3d.lib
  DEL sqlite3.obj
  cl /c /O2 /MD /DSQLITE_USE_URI=1 sqlite3.c
  lib sqlite3.obj
  POPD
)
CALL :DownloadAndExtract "tclap-1.2.5" ^
  "https://github.com/mirror/tclap/archive/v1.2.5.zip" ^
  "95ec0d0904464cb14009b408a62c50a195c1f24ef0921079b8bd034fdd489e28"
SET VIPER_COMMIT="c5adf4c8101d30077fc0fa35cfb1b7b96bf2d1fe"
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
CALL :DownloadAndExtract "yaml-cpp" ^
  "https://github.com/jbeder/yaml-cpp/archive/0f9a586ca1dc29c2ecb8dd715a315b93e3f40f79.zip" ^
  "ff55e0cc373295b8503faf52a5e9569b950d8ec3e704508a62fe9159c37185bc"
IF !BUILD_NEEDED!==1 (
  PUSHD yaml-cpp
  MD build
  PUSHD build
  cmake ..
  cmake --build . --target ALL_BUILD --config Debug
  cmake --build . --target ALL_BUILD --config Release
  POPD
  POPD
)
CALL :DownloadAndExtract "zlib-1.3.1.2" ^
  "https://github.com/madler/zlib/archive/refs/tags/v1.3.1.2.zip" ^
  "2ae5dfd8a1df6cffff4b0cde7cde73f2986aefbaaddc22cc1a36537b0e948afc"
IF !BUILD_NEEDED!==1 (
  PUSHD zlib-1.3.1.2\contrib\vstudio\vc17
  powershell -Command "(Get-Content zlibstat.vcxproj) -replace " ^
    "'ZLIB_WINAPI;', '' -replace " ^
    "'<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>', " ^
    "'<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>' -replace " ^
    "'<RuntimeLibrary>MultiThreaded</RuntimeLibrary>', " ^
    "'<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>' | " ^
    "Set-Content zlibstat.vcxproj"
  msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v143 ^
    /p:Platform=x64 /p:Configuration=Debug
  msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v143 ^
    /p:Platform=x64 /p:Configuration=ReleaseWithoutAsm
  POPD
)
IF "%NUMBER_OF_PROCESSORS%" == "" (
  SET BJAM_PROCESSORS=
) ELSE (
  SET BJAM_PROCESSORS="-j%NUMBER_OF_PROCESSORS%"
)
CALL :DownloadAndExtract "boost_1_90_0" ^
  "https://archives.boost.io/release/1.90.0/source/boost_1_90_0.zip" ^
  "bdc79f179d1a4a60c10fe764172946d0eeafad65e576a8703c4d89d49949973c"
IF !BUILD_NEEDED!==1 (
  PUSHD boost_1_90_0
  PUSHD tools\build
  CALL bootstrap.bat vc143
  POPD
  tools\build\b2 !BJAM_PROCESSORS! --prefix="!ROOT!\boost_1_90_0" ^
    --build-type=complete address-model=64 context-impl=winfib ^
    toolset=msvc-14.3 link=static runtime-link=shared install
  POPD
)
IF !EXIT_STATUS! NEQ 0 (
  EXIT /B !EXIT_STATUS!
)
IF NOT EXIST cache_files (
  MD cache_files
)
>cache_files\beam.txt ECHO !SETUP_HASH!
EXIT /B 0

:DownloadAndExtract
SET FOLDER=%~1
SET URL=%~2
SET EXPECTED_HASH=%~3
SET BUILD_NEEDED=0
FOR /F "tokens=* delims=/" %%A IN ("%URL%") DO (
  SET ARCHIVE=%%~nxA
)
IF EXIST !FOLDER! (
  EXIT /B 0
)
IF NOT EXIST !ARCHIVE! (
  curl -fsL -o "!ARCHIVE!" "!URL!"
  IF ERRORLEVEL 1 (
    ECHO Error: Failed to download !ARCHIVE!.
    SET EXIT_STATUS=1
    EXIT /B
  )
)
FOR /F "skip=1 tokens=*" %%H IN ('certutil -hashfile "!ARCHIVE!" SHA256') DO (
  IF NOT DEFINED ACTUAL_HASH SET ACTUAL_HASH=%%H
)
SET ACTUAL_HASH=!ACTUAL_HASH: =!
IF /I NOT "!ACTUAL_HASH!"=="!EXPECTED_HASH!" (
  ECHO Error: SHA256 mismatch for !ARCHIVE!.
  ECHO   Expected: !EXPECTED_HASH!
  ECHO   Actual:   !ACTUAL_HASH!
  DEL /F /Q "!ARCHIVE!"
  SET EXIT_STATUS=1
  SET ACTUAL_HASH=
  EXIT /B
)
SET ACTUAL_HASH=
SET EXTRACT_PATH=_extract_tmp
RD /S /Q "!EXTRACT_PATH!" >NUL 2>NUL
MD "!EXTRACT_PATH!"
tar -xf "!ARCHIVE!" -C "!EXTRACT_PATH!"
IF ERRORLEVEL 1 (
  ECHO Error: Failed to extract !ARCHIVE!.
  SET EXIT_STATUS=1
  EXIT /B
)
SET DETECTED_FOLDER=
FOR %%F IN ("!EXTRACT_PATH!\*") DO (
  IF "!DETECTED_FOLDER!"=="" (
    SET DETECTED_FOLDER=%%F
  ) ELSE (
    SET DETECTED_FOLDER=MULTIPLE
  )
)
FOR /D %%F IN ("!EXTRACT_PATH!\*") DO (
  IF "!DETECTED_FOLDER!"=="" (
    SET DETECTED_FOLDER=%%F
  ) ELSE (
    SET DETECTED_FOLDER=MULTIPLE
  )
)
IF "!DETECTED_FOLDER!"=="MULTIPLE" (
  REN "!EXTRACT_PATH!" "!FOLDER!"
) ELSE IF NOT "!DETECTED_FOLDER!"=="!EXTRACT_PATH!\!FOLDER!" (
  MOVE /Y "!DETECTED_FOLDER!" "!FOLDER!" >NUL
  RD /S /Q "!EXTRACT_PATH!" >NUL 2>NUL
) ELSE (
  MOVE /Y "!EXTRACT_PATH!\!FOLDER!" "!ROOT!" >NUL
  RD /S /Q "!EXTRACT_PATH!" >NUL 2>NUL
)
SET BUILD_NEEDED=1
DEL /F /Q "!ARCHIVE!"
EXIT /B 0
