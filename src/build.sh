#!/bin/sh

mkdir -p ../bin
pushd ../bin

gcc ../src/main.c -std=c11 -g -o uct

popd
