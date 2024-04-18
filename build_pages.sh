#!/bin/bash
CXX=g++
$CXX example_pages.cpp $1 -g -Wall -fmax-errors=5 -o example_pages -std=c++17 && \
./example_pages
