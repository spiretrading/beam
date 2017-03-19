SETLOCAL
IF [%1] == [] (
  SET config=Release
) ELSE (
  SET config="%1"
)
CALL %~dp0../../Beam/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/AdminClient/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/ClientTemplate/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/DataStoreProfiler/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/HttpFileServer/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/QueryStressTest/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/RegistryServer/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/ServiceLocator/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/ServiceProtocolProfiler/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/ServletTemplate/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/UidServer/Build/Windows/build.bat %config%
CALL %~dp0../../Applications/WebSocketEchoServer/Build/Windows/build.bat %config%
ENDLOCAL
