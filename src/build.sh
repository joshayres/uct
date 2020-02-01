#!/bin/sh

pushd ../bin

gcc ../src/main.c -std=c11 -g -o ugi

popd
