@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET "ROOT=%cd%"
CALL :CheckCache "beam"
IF ERRORLEVEL 1 EXIT /B 0
CALL :SetupVSEnvironment
CALL :AddDependency "Strawberry" ^
  "https://github.com/StrawberryPerl/Perl-Dist-Strawberry/releases/download/SP_54201_64bit/strawberry-perl-5.42.0.1-64bit-portable.zip" ^
  "a1cde185656cf307b51670eed69f648b9eff15b5c518cb136e027c628e650b71"
CALL :AddDependency "cryptopp890" ^
  "https://github.com/weidai11/cryptopp/archive/refs/tags/CRYPTOPP_8_9_0.zip" ^
  "b885403cb13d490bebe90f25fad7150b88857f7acf3bd8b9ca1cec04c9ec8a51" ^
  ":BuildCryptopp"
CALL :AddDependency "mariadb-connector-c-3.4.8" ^
  "https://github.com/mariadb-corporation/mariadb-connector-c/archive/refs/tags/v3.4.8.zip" ^
  "d8d91088ff33bbfe0d469f0ad50f472a39a2a1d9c9d975aa31c0dbbf504e425f" ^
  ":BuildMariaDB"
CALL :AddDependency "openssl-3.6.0" ^
  "https://github.com/openssl/openssl/archive/refs/tags/openssl-3.6.0.zip" ^
  "273d989d1157f0bd494054e1b799b6bdba39d4acaff6dfcb8db02656f1b454dd" ^
  ":BuildOpenSSL"
CALL :AddDependency "sqlite-amalgamation-3510200" ^
  "https://www.sqlite.org/2026/sqlite-amalgamation-3510200.zip" ^
  "6e2a845a493026bdbad0618b2b5a0cf48584faab47384480ed9f592d912f23ec" ^
  ":BuildSQLite"
CALL :AddDependency "tclap-1.2.5" ^
  "https://github.com/mirror/tclap/archive/v1.2.5.zip" ^
  "95ec0d0904464cb14009b408a62c50a195c1f24ef0921079b8bd034fdd489e28"
CALL :AddDependency "yaml-cpp" ^
  "https://github.com/jbeder/yaml-cpp/archive/0f9a586ca1dc29c2ecb8dd715a315b93e3f40f79.zip" ^
  "ff55e0cc373295b8503faf52a5e9569b950d8ec3e704508a62fe9159c37185bc" ^
  ":BuildYamlCpp"
CALL :AddDependency "zlib-1.3.1.2" ^
  "https://github.com/madler/zlib/archive/refs/tags/v1.3.1.2.zip" ^
  "2ae5dfd8a1df6cffff4b0cde7cde73f2986aefbaaddc22cc1a36537b0e948afc" ^
  ":BuildZlib"
CALL :AddDependency "boost_1_90_0" ^
  "https://archives.boost.io/release/1.90.0/source/boost_1_90_0.zip" ^
  "bdc79f179d1a4a60c10fe764172946d0eeafad65e576a8703c4d89d49949973c" ^
  ":BuildBoost"
CALL :AddRepo "aspen" ^
  "https://www.github.com/spiretrading/aspen" ^
  "9ec33ec45ff91ac42b3ea5f1754ae1eabe53d62b" ^
  ":BuildAspen"
CALL :AddRepo "viper" ^
  "https://www.github.com/spiretrading/viper" ^
  "c5adf4c8101d30077fc0fa35cfb1b7b96bf2d1fe"
SET "PATH=!PATH!;!ROOT!\Strawberry\perl\site\bin;!ROOT!\Strawberry\perl\bin;!ROOT!\Strawberry\c\bin"
CALL :InstallDependencies || EXIT /B 1
CALL :InstallRepos || EXIT /B 1
CALL :Commit
EXIT /B !ERRORLEVEL!
ENDLOCAL

:BuildCryptopp
powershell -Command "(Get-Content cryptlib.vcxproj) -replace " ^
  "'<WholeProgramOptimization>true</WholeProgramOptimization>', " ^
  "'<WholeProgramOptimization>false</WholeProgramOptimization>' | " ^
  "Set-Content cryptlib.vcxproj" || EXIT /B 1
powershell -Command "(Get-Content cryptlib.vcxproj) -replace " ^
  "'<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>', " ^
  "'<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>' -replace " ^
  "'<RuntimeLibrary>MultiThreaded</RuntimeLibrary>', " ^
  "'<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>' | " ^
  "Set-Content cryptlib.vcxproj" || EXIT /B 1
msbuild /t:Build /p:UseEnv=True /p:PlatformToolset=v143 /p:Platform=x64 ^
  /p:Configuration=Debug cryptlib.vcxproj || EXIT /B 1
msbuild /t:Build /p:UseEnv=True /p:PlatformToolset=v143 /p:Platform=x64 ^
  /p:Configuration=Release cryptlib.vcxproj || EXIT /B 1
MD include
PUSHD include
MD cryptopp
COPY ..\*.h cryptopp
POPD
EXIT /B 0

:BuildMariaDB
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./mariadb . || EXIT /B 1
PUSHD libmariadb
powershell -Command "(Get-Content mariadbclient.vcxproj) -replace " ^
  "'<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>', " ^
  "'<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>' -replace " ^
  "'<RuntimeLibrary>MultiThreaded</RuntimeLibrary>', " ^
  "'<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>' | " ^
  "Set-Content mariadbclient.vcxproj" || (POPD & EXIT /B 1)
powershell -Command "(Get-Content mariadb_obj.vcxproj) -replace " ^
  "'<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>', " ^
  "'<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>' -replace " ^
  "'<RuntimeLibrary>MultiThreaded</RuntimeLibrary>', " ^
  "'<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>' | " ^
  "Set-Content mariadb_obj.vcxproj" || (POPD & EXIT /B 1)
POPD
cmake --build . --target mariadbclient --config Debug || EXIT /B 1
cmake --build . --target mariadbclient --config Release || EXIT /B 1
EXIT /B 0

:BuildOpenSSL
POPD
MOVE "!FOLDER!" "!FOLDER!-build"
PUSHD "!FOLDER!-build"
perl Configure VC-WIN64A no-asm no-shared no-tests ^
  --prefix="!ROOT!\openssl-3.6.0" --openssldir="!ROOT!\openssl-3.6.0"
SET "CL=/MP"
nmake
nmake install
POPD
RD /S /Q "!FOLDER!-build"
PUSHD "!FOLDER!"
EXIT /B 0

:BuildSQLite
cl /c /Zi /MDd /DSQLITE_USE_URI=1 sqlite3.c || EXIT /B 1
lib sqlite3.obj || EXIT /B 1
COPY sqlite3.lib sqlite3d.lib
DEL sqlite3.obj
cl /c /O2 /MD /DSQLITE_USE_URI=1 sqlite3.c || EXIT /B 1
lib sqlite3.obj || EXIT /B 1
EXIT /B 0

:BuildYamlCpp
MD build
PUSHD build
cmake .. || (POPD & EXIT /B 1)
cmake --build . --target ALL_BUILD --config Debug
cmake --build . --target ALL_BUILD --config Release
POPD
EXIT /B 0

:BuildZlib
PUSHD contrib\vstudio\vc17
powershell -Command "(Get-Content zlibstat.vcxproj) -replace " ^
  "'ZLIB_WINAPI;', '' -replace " ^
  "'<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>', " ^
  "'<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>' -replace " ^
  "'<RuntimeLibrary>MultiThreaded</RuntimeLibrary>', " ^
  "'<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>' | " ^
  "Set-Content zlibstat.vcxproj" || (POPD & EXIT /B 1)
msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v143 ^
  /p:Platform=x64 /p:Configuration=Debug || (POPD & EXIT /B 1)
msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v143 ^
  /p:Platform=x64 /p:Configuration=ReleaseWithoutAsm || (POPD & EXIT /B 1)
POPD
EXIT /B 0

:BuildBoost
IF "%NUMBER_OF_PROCESSORS%"=="" (
  SET "BJAM_PROCESSORS="
) ELSE (
  SET "BJAM_PROCESSORS=-j%NUMBER_OF_PROCESSORS%"
)
PUSHD tools\build
CALL bootstrap.bat vc143 || (POPD & EXIT /B 1)
POPD
tools\build\b2 !BJAM_PROCESSORS! --prefix="!ROOT!\boost_1_90_0" ^
  --build-type=complete address-model=64 context-impl=winfib ^
  toolset=msvc-14.3 link=static runtime-link=shared install || EXIT /B 1
EXIT /B 0

:BuildAspen
CALL configure.bat -DD="!ROOT!" || EXIT /B 1
CALL build.bat Debug || EXIT /B 1
CALL build.bat Release || EXIT /B 1
EXIT /B 0

:CheckCache
SET "CACHE_NAME=%~1"
SET "SETUP_HASH="
FOR /F "skip=1" %%H IN ('certutil -hashfile "%~dp0setup.bat" SHA256') DO (
  IF NOT DEFINED SETUP_HASH SET "SETUP_HASH=%%H"
)
IF EXIST "cache_files\!CACHE_NAME!.txt" (
  SET /P CACHED_HASH=<"cache_files\!CACHE_NAME!.txt"
  IF "!SETUP_HASH!"=="!CACHED_HASH!" EXIT /B 1
)
EXIT /B 0

:Commit
IF NOT EXIST cache_files (
  MD cache_files || EXIT /B 1
)
>"cache_files\!CACHE_NAME!.txt" ECHO !SETUP_HASH!
EXIT /B 0

:SetupVSEnvironment
SET "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /F "usebackq delims=" %%i IN (` ^
    "!VSWHERE!" -prerelease -latest -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat" -arch=amd64
  )
)
EXIT /B 0

:AddDependency
IF NOT DEFINED NEXT_DEPENDENCY_INDEX SET "NEXT_DEPENDENCY_INDEX=0"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].NAME=%~1"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].URL=%~2"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].HASH=%~3"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].BUILD=%~4"
SET /A NEXT_DEPENDENCY_INDEX+=1
EXIT /B 0

:AddRepo
IF NOT DEFINED NEXT_REPO_INDEX SET "NEXT_REPO_INDEX=0"
SET "REPOS[%NEXT_REPO_INDEX%].NAME=%~1"
SET "REPOS[%NEXT_REPO_INDEX%].URL=%~2"
SET "REPOS[%NEXT_REPO_INDEX%].COMMIT=%~3"
SET "REPOS[%NEXT_REPO_INDEX%].BUILD=%~4"
SET /A NEXT_REPO_INDEX+=1
EXIT /B 0

:InstallDependencies
SET "I=0"
:InstallDependenciesLoop
IF NOT DEFINED DEPENDENCIES[%I%].NAME EXIT /B 0
CALL :DownloadAndExtract "!DEPENDENCIES[%I%].NAME!" "!DEPENDENCIES[%I%].URL!" ^
  "!DEPENDENCIES[%I%].HASH!" "!DEPENDENCIES[%I%].BUILD!" || EXIT /B 1
SET /A I+=1
GOTO InstallDependenciesLoop

:InstallRepos
SET "I=0"
:InstallReposLoop
IF NOT DEFINED REPOS[%I%].NAME EXIT /B 0
CALL :CloneOrUpdateRepo "!REPOS[%I%].NAME!" "!REPOS[%I%].URL!" ^
  "!REPOS[%I%].COMMIT!" "!REPOS[%I%].BUILD!" || EXIT /B 1
SET /A I+=1
GOTO InstallReposLoop

:DownloadAndExtract
SET "FOLDER=%~1"
SET "URL=%~2"
SET "EXPECTED_HASH=%~3"
SET "BUILD_LABEL=%~4"
SET "ACTUAL_HASH="
FOR /F "tokens=* delims=/" %%A IN ("!URL!") DO (
  SET "ARCHIVE=%%~nxA"
)
IF EXIST "!FOLDER!" (
  EXIT /B 0
)
IF NOT EXIST "!ARCHIVE!" (
  curl -fsL -o "!ARCHIVE!" "!URL!" || EXIT /B 1
)
FOR /F "skip=1 tokens=*" %%H IN ('certutil -hashfile "!ARCHIVE!" SHA256') DO (
  IF NOT DEFINED ACTUAL_HASH SET "ACTUAL_HASH=%%H"
)
SET "ACTUAL_HASH=!ACTUAL_HASH: =!"
IF /I NOT "!ACTUAL_HASH!"=="!EXPECTED_HASH!" (
  ECHO Error: SHA256 mismatch for !ARCHIVE!.
  ECHO   Expected: !EXPECTED_HASH!
  ECHO   Actual:   !ACTUAL_HASH!
  DEL /F /Q "!ARCHIVE!"
  SET "ACTUAL_HASH="
  EXIT /B 1
)
SET "ACTUAL_HASH="
MD "!FOLDER!" || EXIT /B 1
tar -xf "!ARCHIVE!" -C "!FOLDER!"
IF ERRORLEVEL 1 (
  RD /S /Q "!FOLDER!" >NUL 2>NUL
  EXIT /B 1
)
SET "DIR_COUNT=0"
SET "FILE_COUNT=0"
SET "SINGLE_DIR="
FOR /D %%D IN ("!FOLDER!\*") DO (
  SET /A DIR_COUNT+=1
  SET "SINGLE_DIR=%%~nxD"
)
FOR %%F IN ("!FOLDER!\*") DO (
  SET /A FILE_COUNT+=1
)
IF "!DIR_COUNT!"=="1" IF "!FILE_COUNT!"=="0" (
  FOR /F "delims=" %%D IN ('DIR /AD /B "!FOLDER!\!SINGLE_DIR!" 2^>NUL') DO (
    MOVE "!FOLDER!\!SINGLE_DIR!\%%D" "!FOLDER!" >NUL
  )
  FOR /F "delims=" %%F IN ('DIR /A-D /B "!FOLDER!\!SINGLE_DIR!" 2^>NUL') DO (
    MOVE "!FOLDER!\!SINGLE_DIR!\%%F" "!FOLDER!" >NUL
  )
  RD /S /Q "!FOLDER!\!SINGLE_DIR!" 2>NUL
)
IF DEFINED BUILD_LABEL (
  PUSHD "!FOLDER!"
  CALL !BUILD_LABEL!
  SET "BUILD_RESULT=!ERRORLEVEL!"
  POPD
  IF NOT "!BUILD_RESULT!"=="0" EXIT /B !BUILD_RESULT!
)
DEL /F /Q "!ARCHIVE!"
EXIT /B 0

:CloneOrUpdateRepo
SET "REPO_NAME=%~1"
SET "REPO_URL=%~2"
SET "REPO_COMMIT=%~3"
SET "BUILD_LABEL=%~4"
SET "NEEDS_BUILD=0"
IF NOT EXIST "!REPO_NAME!" (
  git clone "!REPO_URL!" "!REPO_NAME!"
  IF ERRORLEVEL 1 (
    RD /S /Q "!REPO_NAME!" >NUL
    EXIT /B 1
  )
  PUSHD "!REPO_NAME!"
  git checkout "!REPO_COMMIT!"
  POPD
  SET "NEEDS_BUILD=1"
) ELSE (
  PUSHD "!REPO_NAME!"
  git merge-base --is-ancestor "!REPO_COMMIT!" HEAD
  IF ERRORLEVEL 1 (
    git checkout master
    git pull
    git checkout "!REPO_COMMIT!"
    SET "NEEDS_BUILD=1"
  )
  POPD
)
IF "!NEEDS_BUILD!"=="1" (
  IF DEFINED BUILD_LABEL (
    PUSHD "!REPO_NAME!"
    CALL !BUILD_LABEL!
    SET "BUILD_RESULT=!ERRORLEVEL!"
    POPD
    IF NOT "!BUILD_RESULT!"=="0" EXIT /B !BUILD_RESULT!
  )
)
EXIT /B 0
