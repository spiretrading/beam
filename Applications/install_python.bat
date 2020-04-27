@ECHO OFF
IF "%1" == "" (
  SET CONFIG=Release
) ELSE (
  SET CONFIG=%1
)
FOR /f "delims=" %%i IN ('python -m site --user-site') DO SET PYTHON_PATH="%%i"
PUSHD ..\Beam\Dependencies\aspen
CALL install_python.bat %*
POPD
MD %PYTHON_PATH%\beam
COPY Python\__init__.py %PYTHON_PATH%\beam
COPY ..\Beam\Libraries\%CONFIG%\_beam.pyd %PYTHON_PATH%\beam
