IF "%1" == "" (
  SET CONFIG=Release
) ELSE (
  SET CONFIG=%1
)
FOR /f "delims=" %%i IN ('python -m site --user-site') DO SET PYTHON_PATH="%%i"
CALL ..\Beam\Dependencies\aspen\install_python.bat %*
MD %PYTHON_PATH%\beam
COPY Python\__init__.py %PYTHON_PATH%\beam
COPY ..\Beam\Libraries\%CONFIG%\_beam.pyd %PYTHON_PATH%\beam
