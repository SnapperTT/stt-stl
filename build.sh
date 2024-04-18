#!/bin/bash
CXX=g++
$CXX example.cpp -O3 $1 -g -Wall -fmax-errors=5 -o example -std=c++17 && \
./example
