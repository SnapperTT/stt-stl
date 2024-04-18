#!/bin/bash
CXX=g++
OPT=
$CXX example.cpp $1 $OPT -g -Wall -fmax-errors=5 -o example -std=c++17 ../lib_lin_x64/liblua.a && \
./example

