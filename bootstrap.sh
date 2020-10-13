#!/bin/bash

git submodule update --init --recursive

pushd AFL
make -j4
popd

pushd e9patch-github
./build.sh
popd
