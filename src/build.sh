#!/bin/zsh

../../ctime/ctime -begin uct.ctm

mkdir -p ../bin
pushd ../bin

gcc ../src/main.c -std=c11 -g -o uct

popd

../../ctime/ctime -end uct.ctm