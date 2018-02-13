#!/bin/bash
pushd ../../
rm -rf library
node ./node_modules/webpack/bin/webpack.js
popd
