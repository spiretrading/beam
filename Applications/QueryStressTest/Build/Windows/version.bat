cd %~dp0../..
mkdir Include
cd Include
mkdir QueryStressTest
cd %~dp0
printf "#define QUERY_STRESS_TEST_VERSION """> %~dp0../../Include/QueryStressTest/Version.hpp
hg id -n | tr -d "\n\" >> %~dp0../../Include/QueryStressTest/Version.hpp
printf """" >> %~dp0../../Include/QueryStressTest/Version.hpp
