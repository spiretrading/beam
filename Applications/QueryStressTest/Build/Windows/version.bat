pushd %~dp0..\..
mkdir Include
pushd Include
mkdir QueryStressTest
popd
popd
printf "#define QUERY_STRESS_TEST_VERSION """> %~dp0../../Include/QueryStressTest/Version.hpp
git rev-list --count --first-parent HEAD | tr -d "\n\" >> %~dp0../../Include/QueryStressTest/Version.hpp
printf """" >> %~dp0../../Include/QueryStressTest/Version.hpp
printf "\n" >> %~dp0../../Include/QueryStressTest/Version.hpp
