#!/bin/bash -e

rm -rf CMakeCache.txt CMakeFiles/

cmake -DWerror=on .

make clean
make
make check
