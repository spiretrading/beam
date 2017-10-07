pushd %~dp0..\..
mkdir Include
pushd Include
mkdir ServletTemplate
popd
popd
printf "#define SERVLET_TEMPLATE_VERSION """> %~dp0../../Include/ServletTemplate/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/ServletTemplate/Version.hpp
printf """" >> %~dp0../../Include/ServletTemplate/Version.hpp
printf "\n" >> %~dp0../../Include/ServletTemplate/Version.hpp
