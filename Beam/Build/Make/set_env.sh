#!/bin/bash
set -o errexit
set -o pipefail
beam_local_path=$(pwd)
if [ "$(uname -s)" = "Darwin" ]
then
  so_ext="dylib"
else
  so_ext="so"
fi
export BEAM_INCLUDE_PATH=$beam_local_path/../../Include
export BEAM_PYTHON_LIBRARY_DEBUG_PATH=$beam_local_path/../../Library/Debug/beam.$so_ext
export BEAM_PYTHON_LIBRARY_OPTIMIZED_PATH=$beam_local_path/../../Library/Release/beam.$so_ext
export BOOST_INCLUDE_PATH=/usr/local/include
export BOOST_DEBUG_PATH=/usr/local/lib
export BOOST_OPTIMIZED_PATH=/usr/local/lib
export BOOST_CHRONO_LIBRARY_DEBUG_PATH=$BOOST_DEBUG_PATH/libboost_chrono.a
export BOOST_CHRONO_LIBRARY_OPTIMIZED_PATH=$BOOST_OPTIMIZED_PATH/libboost_chrono.a
export BOOST_CONTEXT_LIBRARY_DEBUG_PATH=$BOOST_DEBUG_PATH/libboost_context.a
export BOOST_CONTEXT_LIBRARY_OPTIMIZED_PATH=$BOOST_OPTIMIZED_PATH/libboost_context.a
export BOOST_DATE_TIME_LIBRARY_DEBUG_PATH=$BOOST_DEBUG_PATH/libboost_date_time.a
export BOOST_DATE_TIME_LIBRARY_OPTIMIZED_PATH=$BOOST_OPTIMIZED_PATH/libboost_date_time.a
export BOOST_FILE_SYSTEM_LIBRARY_DEBUG_PATH=$BOOST_DEBUG_PATH/libboost_filesystem.a
export BOOST_FILE_SYSTEM_LIBRARY_OPTIMIZED_PATH=$BOOST_OPTIMIZED_PATH/libboost_filesystem.a
export BOOST_PYTHON_LIBRARY_DEBUG_PATH=$BOOST_DEBUG_PATH/libboost_python.a
export BOOST_PYTHON_LIBRARY_OPTIMIZED_PATH=$BOOST_OPTIMIZED_PATH/libboost_python.a
export BOOST_REGEX_LIBRARY_DEBUG_PATH=$BOOST_DEBUG_PATH/libboost_regex.a
export BOOST_REGEX_LIBRARY_OPTIMIZED_PATH=$BOOST_OPTIMIZED_PATH/libboost_regex.a
export BOOST_SYSTEM_LIBRARY_DEBUG_PATH=$BOOST_DEBUG_PATH/libboost_system.a
export BOOST_SYSTEM_LIBRARY_OPTIMIZED_PATH=$BOOST_OPTIMIZED_PATH/libboost_system.a
export BOOST_THREAD_LIBRARY_DEBUG_PATH=$BOOST_DEBUG_PATH/libboost_thread.a
export BOOST_THREAD_LIBRARY_OPTIMIZED_PATH=$BOOST_OPTIMIZED_PATH/libboost_thread.a
export CPPUNIT_INCLUDE_PATH=/usr/local/include
export CPPUNIT_LIBRARY_DEBUG_PATH=/usr/local/lib/libcppunit.a
export CPPUNIT_LIBRARY_OPTIMIZED_PATH=/usr/local/lib/libcppunit.a
export CRYPTOPP_INCLUDE_PATH=/usr/local/include
export CRYPTOPP_LIBRARY_DEBUG_PATH=/usr/local/lib/libcryptopp.a
export CRYPTOPP_LIBRARY_OPTIMIZED_PATH=/usr/local/lib/libcryptopp.a
export LUA_INCLUDE_PATH=/usr/local/include
export LUA_LIBRARY_DEBUG_PATH=/usr/local/lib/liblua.a
export LUA_LIBRARY_OPTIMIZED_PATH=/usr/local/lib/liblua.a
export MYSQL_INCLUDE_PATH=/usr/local/mysql/include
export MYSQL_LIBRARY_DEBUG_PATH=/usr/local/mysql/lib/libmysqlclient.a
export MYSQL_LIBRARY_OPTIMIZED_PATH=/usr/local/mysql/lib/libmysqlclient.a
export MYSQLPP_INCLUDE_PATH=/usr/local/include
export MYSQLPP_LIBRARY_DEBUG_PATH=/usr/local/lib/libmysqlpp.$so_ext
export MYSQLPP_LIBRARY_OPTIMIZED_PATH=/usr/local/lib/libmysqlpp.$so_ext
export OPEN_SSL_INCLUDE_PATH=/usr/local/ssl/include
export OPEN_SSL_BASE_LIBRARY_DEBUG_PATH=/usr/local/ssl/lib/libcrypto.a
export OPEN_SSL_BASE_LIBRARY_OPTIMIZED_PATH=/usr/local/ssl/lib/libcrypto.a
export OPEN_SSL_LIBRARY_DEBUG_PATH=/usr/local/ssl/lib/libssl.a
export OPEN_SSL_LIBRARY_OPTIMIZED_PATH=/usr/local/ssl/lib/libssl.a
export PYTHON_INCLUDE_PATH=/usr/include/python2.7
export PYTHON_LIBRARY_DEBUG_PATH=/usr/lib/python2.7/config-x86_64-linux-gnu/libpython2.7.$so_ext
export PYTHON_LIBRARY_OPTIMIZED_PATH=/usr/lib/python2.7/config-x86_64-linux-gnu/libpython2.7.$so_ext
export TCLAP_INCLUDE_PATH=/usr/local/include
export YAML_INCLUDE_PATH=/usr/local/include/
export YAML_LIBRARY_DEBUG_PATH=/usr/local/lib/libyaml-cpp.a
export YAML_LIBRARY_OPTIMIZED_PATH=/usr/local/lib/libyaml-cpp.a
export ZLIB_INCLUDE_PATH=/usr/local/include/zlib
export ZLIB_LIBRARY_DEBUG_PATH=/usr/local/lib/libz.a
export ZLIB_LIBRARY_OPTIMIZED_PATH=/usr/local/lib/libz.a
