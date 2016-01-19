cd %~dp0../..
mkdir Include
cd Include
mkdir ServletTemplate
cd %~dp0
printf "#define SERVLET_TEMPLATE_VERSION """> %~dp0../../Include/ServletTemplate/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/ServletTemplate/Version.hpp
printf """" >> %~dp0../../Include/ServletTemplate/Version.hpp
printf "\n" >> %~dp0../../Include/ServletTemplate/Version.hpp
