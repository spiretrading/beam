@ECHO OFF
SETLOCAL
printf "#define CLIENT_TEMPLATE_VERSION """> Version.hpp
git --git-dir=%~dp0..\..\.git rev-list --count --first-parent HEAD | tr -d "\n\" >> Version.hpp
printf """" >> Version.hpp
printf "\n" >> Version.hpp
ENDLOCAL