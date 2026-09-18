@ECHO OFF
SETLOCAL EnableDelayedExpansion
FOR /F "delims==" %%V IN ('SET DEPENDENCIES[ 2^>NUL') DO (
  SET "%%V="
)
SET "NEXT_DEPENDENCY_INDEX=0"
FOR /F "delims==" %%V IN ('SET REPOS[ 2^>NUL') DO (
  SET "%%V="
)
SET "NEXT_REPO_INDEX=0"
SET "SETUP_HASH="
FOR /F "skip=1" %%H IN ('certutil -hashfile "%~dp0setup.bat" SHA256') DO (
  IF NOT DEFINED SETUP_HASH SET "SETUP_HASH=%%H"
)
IF NOT DEFINED SETUP_HASH EXIT /B 1
SET "ROOT=%cd%"
SET "CACHE_DIRECTORY=!ROOT!\cache_files\beam"
IF NOT EXIST "!CACHE_DIRECTORY!" (
  MD "!CACHE_DIRECTORY!" || EXIT /B 1
)
CALL :SetupVSEnvironment || EXIT /B 1
SET "PERL_URL=https://github.com/StrawberryPerl/Perl-Dist-Strawberry"
SET "PERL_URL=!PERL_URL!/releases/download/SP_54201_64bit"
CALL :AddDependency "Strawberry" ^
  "!PERL_URL!/strawberry-perl-5.42.0.1-64bit-portable.zip" ^
  "a1cde185656cf307b51670eed69f648b9eff15b5c518cb136e027c628e650b71" "" 0
CALL :AddDependency "cryptopp890" ^
  "https://github.com/weidai11/cryptopp/archive/b524266.zip" ^
  "51959987cc4d22289525b916dfc1b7239a956c2b903f9fa41ef4cde6e49a016a" ^
  ":BuildCryptopp"
CALL :AddDependency "openssl-3.6.0-build" ^
  "https://github.com/openssl/openssl/archive/refs/tags/openssl-3.6.0.zip" ^
  "273d989d1157f0bd494054e1b799b6bdba39d4acaff6dfcb8db02656f1b454dd" ^
  ":BuildOpenSSL"
CALL :AddDependency "tclap-1.4.0-rc2" ^
  "https://downloads.sourceforge.net/project/tclap/tclap-1.4.0-rc2.tar.bz2" ^
  "ca52ce5badc477aeda59866601aad85c55e014c5400c15ed13e21fe7d0c1c5f7"
CALL :AddDependency "yaml-cpp" ^
  "https://github.com/jbeder/yaml-cpp/archive/refs/tags/yaml-cpp-0.9.0.zip" ^
  "1c22709eb1fcde200c87ef4e878ce2c9477cc05eae84ebf1f72ec5b356468fee" ^
  ":BuildYamlCpp"
CALL :AddDependency "zlib-1.3.1.2" ^
  "https://github.com/madler/zlib/archive/refs/tags/v1.3.1.2.zip" ^
  "2ae5dfd8a1df6cffff4b0cde7cde73f2986aefbaaddc22cc1a36537b0e948afc" ^
  ":BuildZlib"
CALL :AddDependency "boost_1_91_0" ^
  "https://archives.boost.io/release/1.91.0/source/boost_1_91_0.zip" ^
  "69c6f32fbda3c478fb310ec251e6699e5e584dbc71afb5425c2f6e98c9540a77" ^
  ":BuildBoost"
CALL :AddRepo "aspen" ^
  "https://www.github.com/spiretrading/aspen" ^
  "2fd3dd5ad32a7936a41a895806714acd0c7b0ebb" ^
  ":BuildAspen"
CALL :AddRepo "viper" ^
  "https://www.github.com/spiretrading/viper" ^
  "955b013ef66386e8b9a5656d472604eb213e8319" ^
  ":BuildViper"
SET "PATH=!ROOT!\Strawberry\perl\bin;!PATH!"
SET "PATH=!ROOT!\Strawberry\perl\site\bin;!PATH!"
SET "PATH=!PATH!;!ROOT!\Strawberry\c\bin"
CALL :InstallDependencies || EXIT /B 1
CALL :InstallRepos || EXIT /B 1
EXIT /B 0
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
msbuild /t:Build /p:UseEnv=True /p:PlatformToolset=v145 /p:Platform=x64 ^
  /p:Configuration=Debug cryptlib.vcxproj || EXIT /B 1
msbuild /t:Build /p:UseEnv=True /p:PlatformToolset=v145 /p:Platform=x64 ^
  /p:Configuration=Release cryptlib.vcxproj || EXIT /B 1
IF NOT EXIST include\cryptopp MD include\cryptopp || EXIT /B 1
COPY /Y *.h include\cryptopp >NUL || EXIT /B 1
EXIT /B 0

:BuildOpenSSL
SETLOCAL
perl Configure VC-WIN64A no-asm no-shared no-tests ^
  --prefix="!ROOT!\openssl-3.6.0" ^
  --openssldir="!ROOT!\openssl-3.6.0" || (ENDLOCAL & EXIT /B 1)
SET "CL=/MP"
nmake || (ENDLOCAL & EXIT /B 1)
nmake install || (ENDLOCAL & EXIT /B 1)
ENDLOCAL
EXIT /B 0

:BuildYamlCpp
SETLOCAL
SET "CMAKE_GENERATOR="
SET "CMAKE_GENERATOR_PLATFORM="
SET "CMAKE_GENERATOR_TOOLSET="
SET "CMAKE_GENERATOR_INSTANCE="
cmake --fresh -S . -B build -A x64 -DYAML_CPP_BUILD_TESTS=OFF ^
  -DYAML_CPP_BUILD_TOOLS=OFF || (ENDLOCAL & EXIT /B 1)
FOR %%C IN (Debug Release) DO (
  cmake --build build --target yaml-cpp --config %%C || (
    ENDLOCAL
    EXIT /B 1
  )
)
ENDLOCAL
EXIT /B 0

:BuildZlib
PUSHD contrib\vstudio\vc17 || EXIT /B 1
powershell -Command "(Get-Content zlibstat.vcxproj) -replace " ^
  "'ZLIB_WINAPI;', '' -replace " ^
  "'<RuntimeLibrary>MultiThreadedDebug</RuntimeLibrary>', " ^
  "'<RuntimeLibrary>MultiThreadedDebugDLL</RuntimeLibrary>' -replace " ^
  "'<RuntimeLibrary>MultiThreaded</RuntimeLibrary>', " ^
  "'<RuntimeLibrary>MultiThreadedDLL</RuntimeLibrary>' | " ^
  "Set-Content zlibstat.vcxproj" || (POPD & EXIT /B 1)
msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v145 ^
  /p:Platform=x64 /p:Configuration=Debug || (POPD & EXIT /B 1)
msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v145 ^
  /p:Platform=x64 /p:Configuration=ReleaseWithoutAsm || (POPD & EXIT /B 1)
POPD
EXIT /B 0

:BuildBoost
IF "%NUMBER_OF_PROCESSORS%"=="" (
  SET "BJAM_PROCESSORS="
) ELSE (
  SET "BJAM_PROCESSORS=-j%NUMBER_OF_PROCESSORS%"
)
PUSHD tools\build || EXIT /B 1
CALL bootstrap.bat vc145 || (POPD & EXIT /B 1)
POPD
tools\build\b2 !BJAM_PROCESSORS! --prefix="!ROOT!\boost_1_91_0" ^
  --build-type=complete address-model=64 context-impl=winfib cxxstd=latest ^
  define=_WIN32_WINNT=0x0A00 toolset=msvc-14.5 link=static ^
  runtime-link=shared install || EXIT /B 1
EXIT /B 0

:BuildViper
PUSHD "!ROOT!" || EXIT /B 1
CALL "!ROOT!\viper\setup.bat" || (POPD & EXIT /B 1)
POPD
EXIT /B 0

:BuildAspen
CALL "!ROOT!\aspen\configure.bat" -DD="!ROOT!" || EXIT /B 1
CALL "!ROOT!\aspen\build.bat" Debug || EXIT /B 1
CALL "!ROOT!\aspen\build.bat" Release || EXIT /B 1
EXIT /B 0

:SetupVSEnvironment
SET "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
SET "VS_FOUND="
FOR /F "usebackq delims=" %%i IN (` ^
    "!VSWHERE!" -prerelease -latest -products * ^
      -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 ^
      -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat" -no_logo -arch=x64 -host_arch=x64 || (
      EXIT /B 1
    )
    SET "VS_FOUND=1"
  )
)
IF NOT DEFINED VS_FOUND (
  ECHO Error: Visual Studio C++ build tools were not found.
  EXIT /B 1
)
EXIT /B 0

:AddDependency
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].NAME=%~1"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].URL=%~2"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].HASH=%~3"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].BUILD=%~4"
SET "DEPENDENCIES[%NEXT_DEPENDENCY_INDEX%].STRIP=%~5"
SET /A NEXT_DEPENDENCY_INDEX+=1
EXIT /B 0

:AddRepo
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
  "!DEPENDENCIES[%I%].HASH!" "!DEPENDENCIES[%I%].BUILD!" ^
  "!DEPENDENCIES[%I%].STRIP!" || EXIT /B 1
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
SET "BUILD_MARKER=!CACHE_DIRECTORY!\!FOLDER!.build_complete"
SET "URL=%~2"
SET "EXPECTED_HASH=%~3"
SET "BUILD_HASH=!EXPECTED_HASH! !SETUP_HASH!"
SET "BUILD_LABEL=%~4"
SET "STRIP=%~5"
IF NOT DEFINED STRIP SET "STRIP=1"
SET "ACTUAL_HASH="
FOR /F "tokens=* delims=/" %%A IN ("!URL!") DO (
  SET "ARCHIVE=%%~nxA"
)
SET "CACHED_HASH="
IF EXIST "!BUILD_MARKER!" (
  SET /P CACHED_HASH=<"!BUILD_MARKER!"
  IF EXIST "!FOLDER!\" (
    IF "!CACHED_HASH!"=="!BUILD_HASH!" EXIT /B 0
  )
  DEL /F /Q "!BUILD_MARKER!"
  IF EXIST "!BUILD_MARKER!" EXIT /B 1
)
IF EXIST "!FOLDER!\.beam_extract_complete" (
  SET /P CACHED_HASH=<"!FOLDER!\.beam_extract_complete"
  IF "!CACHED_HASH!"=="!EXPECTED_HASH!" GOTO BuildDependency
  DEL /F /Q "!FOLDER!\.beam_extract_complete"
  IF EXIST "!FOLDER!\.beam_extract_complete" EXIT /B 1
)
IF NOT EXIST "!ARCHIVE!" (
  curl -fsL -o "!ARCHIVE!" "!URL!" || (
    IF EXIST "!ARCHIVE!" DEL /F /Q "!ARCHIVE!"
    EXIT /B 1
  )
)
FOR /F "skip=1 tokens=*" %%H IN ('certutil -hashfile "!ARCHIVE!" SHA256') DO (
  IF NOT DEFINED ACTUAL_HASH SET "ACTUAL_HASH=%%H"
)
SET "ACTUAL_HASH=!ACTUAL_HASH: =!"
IF /I NOT "!ACTUAL_HASH!"=="!EXPECTED_HASH!" (
  ECHO Error: SHA256 mismatch for !ARCHIVE!.
  DEL /F /Q "!ARCHIVE!"
  EXIT /B 1
)
IF NOT EXIST "!FOLDER!" (
  MD "!FOLDER!" || EXIT /B 1
)
cmake -DARCHIVE:FILEPATH="!ARCHIVE!" -DDESTINATION:PATH="!FOLDER!" ^
  -DSTRIP_COMPONENTS=!STRIP! -P "%~dp0Config\extract.cmake" || EXIT /B 1
(ECHO !EXPECTED_HASH!) >"!FOLDER!\.beam_extract_complete" || EXIT /B 1
:BuildDependency
IF DEFINED BUILD_LABEL (
  PUSHD "!FOLDER!" || EXIT /B 1
  CALL !BUILD_LABEL!
  SET "BUILD_RESULT=!ERRORLEVEL!"
  POPD
  IF NOT "!BUILD_RESULT!"=="0" EXIT /B !BUILD_RESULT!
)
(ECHO !BUILD_HASH!) >"!BUILD_MARKER!" || EXIT /B 1
IF EXIST "!ARCHIVE!" DEL /F /Q "!ARCHIVE!"
EXIT /B 0

:CloneOrUpdateRepo
SET "REPO_NAME=%~1"
SET "BUILD_MARKER=!CACHE_DIRECTORY!\!REPO_NAME!.build_complete"
SET "REPO_URL=%~2"
SET "REPO_COMMIT=%~3"
SET "BUILD_LABEL=%~4"
SET "IS_NEW_REPO="
IF NOT EXIST "!REPO_NAME!" (
  IF EXIST "!BUILD_MARKER!" (
    DEL /F /Q "!BUILD_MARKER!"
    IF EXIST "!BUILD_MARKER!" EXIT /B 1
  )
  git clone "!REPO_URL!" "!REPO_NAME!" || EXIT /B 1
  SET "IS_NEW_REPO=1"
)
PUSHD "!REPO_NAME!" || EXIT /B 1
IF DEFINED IS_NEW_REPO (
  git checkout "!REPO_COMMIT!" || (POPD & EXIT /B 1)
)
git merge-base --is-ancestor "!REPO_COMMIT!" HEAD >NUL 2>NUL
IF ERRORLEVEL 1 (
  git fetch origin || (POPD & EXIT /B 1)
  IF EXIST "!BUILD_MARKER!" (
    DEL /F /Q "!BUILD_MARKER!"
    IF EXIST "!BUILD_MARKER!" (POPD & EXIT /B 1)
  )
  git checkout "!REPO_COMMIT!" || (POPD & EXIT /B 1)
)
SET "REPO_HEAD="
FOR /F %%H IN ('git rev-parse HEAD') DO (
  SET "REPO_HEAD=%%H"
)
IF NOT DEFINED REPO_HEAD (POPD & EXIT /B 1)
SET "BUILD_HASH=!REPO_HEAD! !SETUP_HASH!"
SET "CACHED_HASH="
IF EXIST "!BUILD_MARKER!" (
  SET /P CACHED_HASH=<"!BUILD_MARKER!"
)
IF NOT "!CACHED_HASH!"=="!BUILD_HASH!" (
  IF EXIST "!BUILD_MARKER!" (
    DEL /F /Q "!BUILD_MARKER!"
    IF EXIST "!BUILD_MARKER!" (POPD & EXIT /B 1)
  )
  IF DEFINED BUILD_LABEL (
    CALL !BUILD_LABEL!
    SET "BUILD_RESULT=!ERRORLEVEL!"
    IF NOT "!BUILD_RESULT!"=="0" (POPD & EXIT /B !BUILD_RESULT!)
  )
  (ECHO !BUILD_HASH!) >"!BUILD_MARKER!" || (POPD & EXIT /B 1)
) ELSE (
  PUSHD "!ROOT!" || (POPD & EXIT /B 1)
  CALL "!ROOT!\!REPO_NAME!\setup.bat"
  SET "BUILD_RESULT=!ERRORLEVEL!"
  POPD
  IF NOT "!BUILD_RESULT!"=="0" (POPD & EXIT /B !BUILD_RESULT!)
)
POPD
EXIT /B 0
