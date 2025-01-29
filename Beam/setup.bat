@ECHO OFF
SETLOCAL EnableDelayedExpansion
SET EXIT_STATUS=0
SET ROOT=%cd%
IF EXIST cache_files\beam.txt (
  powershell -Command "& { "^
    "$setupTimestamp = (Get-Item '%~dp0setup.bat').LastWriteTime; "^
    "$aspenTimestamp = (Get-Item 'cache_files\\beam.txt').LastWriteTime; "^
    "if ($setupTimestamp -lt $aspenTimestamp) {"^
    "  Exit 0;"^
    "} else {"^
    "  Exit 1;"^
    "}"^
  "}"
  IF ERRORLEVEL 0 (
    EXIT /B 0
  )
)
SET VSWHERE="%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
FOR /F "usebackq delims=" %%i IN (` ^
    !VSWHERE! -prerelease -latest -property installationPath`) DO (
  IF EXIST "%%i\Common7\Tools\vsdevcmd.bat" (
    CALL "%%i\Common7\Tools\vsdevcmd.bat" -arch=amd64
  )
)
CALL :DownloadAndExtract "Strawberry" ^
  "https://github.com/StrawberryPerl/Perl-Dist-Strawberry/releases/download/SP_54001_64bit_UCRT/strawberry-perl-5.40.0.1-64bit-portable.zip"
SET PATH=!PATH!;!ROOT!\Strawberry\perl\site\bin;!ROOT!\Strawberry\perl\bin;!ROOT!\Strawberry\c\bin
SET BUILD_ASPEN=
SET ASPEN_COMMIT="7991dbf01d93c702fd1638150dbd59338be95e40"
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
  "https://github.com/weidai11/cryptopp/archive/refs/tags/CRYPTOPP_8_9_0.zip"
IF %BUILD_NEEDED%==1 (
  PUSHD cryptopp890
  TYPE cryptlib.vcxproj | sed "s/<WholeProgramOptimization>true<\/WholeProgramOptimization>/<WholeProgramOptimization>false<\/WholeProgramOptimization>/" > cryptlib.vcxproj.new
  MOVE cryptlib.vcxproj.new cryptlib.vcxproj
  TYPE cryptlib.vcxproj | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > cryptlib.vcxproj.new
  MOVE cryptlib.vcxproj.new cryptlib.vcxproj
  msbuild /t:Build /p:UseEnv=True /p:PlatformToolset=v143 /p:Platform=x64 /p:Configuration=Debug cryptlib.vcxproj
  msbuild /t:Build /p:UseEnv=True /p:PlatformToolset=v143 /p:Platform=x64 /p:Configuration=Release cryptlib.vcxproj
  MD include
  PUSHD include
  MD cryptopp
  COPY ..\*.h cryptopp
  POPD
  POPD
)
IF NOT EXIST mariadb-connector-c-3.4.3 (
  wget https://github.com/mariadb-corporation/mariadb-connector-c/archive/refs/tags/v3.4.3.zip -O mariadb-connector-c-3.4.3.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    tar -xf mariadb-connector-c-3.4.3.zip
    PUSHD mariadb-connector-c-3.4.3
    cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=./mariadb .
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
  DEL /F /Q mariadb-connector-c-3.4.3.zip
)
IF NOT EXIST openssl-3.4.0 (
  wget https://github.com/openssl/openssl/releases/download/openssl-3.4.0/openssl-3.4.0.tar.gz -O openssl-3.4.0.tar.gz --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    gzip -d -c openssl-3.4.0.tar.gz | tar -xf -
    MOVE openssl-3.4.0 openssl-3.4.0-build
    PUSHD openssl-3.4.0-build
    perl Configure VC-WIN64A no-asm no-shared no-tests --prefix="!ROOT!\openssl-3.4.0" --openssldir="!ROOT!\openssl-3.4.0"
    SET CL=/MP
    nmake
    nmake install
    POPD
    RD /S /Q openssl-3.4.0-build
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q openssl-3.4.0.tar.gz
)
IF NOT EXIST sqlite-amalgamation-3480000 (
  wget https://www.sqlite.org/2025/sqlite-amalgamation-3480000.zip -O sqlite-amalgamation-3480000.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    tar -xf sqlite-amalgamation-3480000.zip
    PUSHD sqlite-amalgamation-3480000
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
  DEL /F /Q sqlite-amalgamation-3480000.zip
)
CALL :DownloadAndExtract "tclap-1.2.5" ^
  "https://github.com/mirror/tclap/archive/v1.2.5.zip"
SET VIPER_COMMIT="baa7791140abd87b16d2132451812e293e71c93d"
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
    cmake ..
    cmake --build . --target ALL_BUILD --config Debug
    cmake --build . --target ALL_BUILD --config Release
    POPD
    POPD
  ) ELSE (
    RD /S /Q yaml-cpp
    SET EXIT_STATUS=1
  )
)
IF NOT EXIST zlib-1.3.1 (
  git clone --branch v1.3.1 https://github.com/madler/zlib.git zlib-1.3.1
  IF !ERRORLEVEL! EQU 0 (
    PUSHD zlib-1.3.1\contrib\vstudio\vc17
    TYPE zlibstat.vcxproj | sed "s/ZLIB_WINAPI;//" | sed "s/<RuntimeLibrary>MultiThreadedDebug<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDebugDLL<\/RuntimeLibrary>/" | sed "s/<RuntimeLibrary>MultiThreaded<\/RuntimeLibrary>/<RuntimeLibrary>MultiThreadedDLL<\/RuntimeLibrary>/" > zlibstat.vcxproj.new
    MOVE zlibstat.vcxproj.new zlibstat.vcxproj
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v143 /p:Platform=x64 /p:Configuration=Debug
    msbuild zlibstat.vcxproj /p:UseEnv=True /p:PlatformToolset=v143 /p:Platform=x64 /p:Configuration=ReleaseWithoutAsm
    POPD
  ) ELSE (
    RD /S /Q zlib-1.3.1
    SET EXIT_STATUS=1
  )
)
IF "%NUMBER_OF_PROCESSORS%" == "" (
  SET BJAM_PROCESSORS=
) ELSE (
  SET BJAM_PROCESSORS="-j%NUMBER_OF_PROCESSORS%"
)
IF NOT EXIST boost_1_87_0 (
  wget https://archives.boost.io/release/1.87.0/source/boost_1_87_0.zip -O boost_1_87_0.zip --no-check-certificate
  IF !ERRORLEVEL! LEQ 0 (
    tar -xf boost_1_87_0.zip
    PUSHD boost_1_87_0
    PUSHD tools\build
    CALL bootstrap.bat vc143
    POPD
    tools\build\b2 !BJAM_PROCESSORS! --without-context --prefix="!ROOT!\boost_1_87_0" --build-type=complete toolset=msvc-14.3 link=static,shared runtime-link=shared install
    tools\build\b2 !BJAM_PROCESSORS! --with-context --prefix="!ROOT!\boost_1_87_0" --build-type=complete toolset=msvc-14.3 link=static runtime-link=shared install
    POPD
  ) ELSE (
    SET EXIT_STATUS=1
  )
  DEL /F /Q boost_1_87_0.zip
)
IF NOT EXIST cache_files (
  MD cache_files
)
ECHO timestamp > cache_files\beam.txt
ENDLOCAL
EXIT /B !EXIT_STATUS!

:DownloadAndExtract
SET FOLDER=%~1
SET URL=%~2
SET BUILD_NEEDED=0
FOR /F "tokens=* delims=/" %%A IN ("%URL%") DO (
  SET ARCHIVE=%%~nxA
)
SET EXTENSION=%ARCHIVE:~-4%
IF EXIST %FOLDER% (
  EXIT /B 0
)
powershell -Command "Invoke-WebRequest -Uri '%URL%' -OutFile '%ARCHIVE%'"
IF ERRORLEVEL 1 (
  ECHO Error: Failed to download %ARCHIVE%.
  SET EXIT_STATUS=1
  EXIT /B
)
SET EXTRACT_PATH=_extract_tmp
MD "%EXTRACT_PATH%"
IF /I "%EXTENSION%"==".zip" (
  powershell -Command ^
    "Expand-Archive -Path '%ARCHIVE%' -DestinationPath '%EXTRACT_PATH%'"
) ELSE IF /I "%EXTENSION%"==".tgz" (
  powershell -Command "tar -xf '%ARCHIVE%' -C '%EXTRACT_PATH%'"
) ELSE IF /I "%ARCHIVE:~-7%"==".tar.gz" (
  powershell -Command "tar -xf '%ARCHIVE%' -C '%EXTRACT_PATH%'"
) ELSE (
  ECHO Error: Unknown archive format for %ARCHIVE%.
  SET EXIT_STATUS=1
  EXIT /B 1
)
SET DETECTED_FOLDER=
FOR %%F IN ("%EXTRACT_PATH%\*") DO (
  IF NOT DEFINED DETECTED_FOLDER (
    SET DETECTED_FOLDER=%%F
  ) ELSE (
    SET DETECTED_FOLDER=MULTIPLE
  )
)
IF "%DETECTED_FOLDER%"=="MULTIPLE" (
  MD "%FOLDER%"
  MOVE "%EXTRACT_PATH%\*" "%FOLDER%\"
) ELSE IF NOT "%DETECTED_FOLDER%"=="" (
  IF NOT "%DETECTED_FOLDER%"=="MULTIPLE" (
    IF NOT "%DETECTED_FOLDER%"=="%FOLDER%" (
      MOVE "%DETECTED_FOLDER%" "%FOLDER%"
    )
  )
)
RD /S /Q "%EXTRACT_PATH%"
IF ERRORLEVEL 1 (
  ECHO Error: Failed to extract %ARCHIVE%.
  SET EXIT_STATUS=1
  EXIT /B 0
)
SET BUILD_NEEDED=1
DEL /F /Q %ARCHIVE%
EXIT /B 0
