cd %~dp0../..
mkdir Include
cd Include
mkdir ClientTemplate
cd %~dp0
printf "#define CLIENT_TEMPLATE_VERSION """> %~dp0../../Include/ClientTemplate/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/ClientTemplate/Version.hpp
printf """" >> %~dp0../../Include/ClientTemplate/Version.hpp
