#!/bin/bash -e

rm -rf CMakeCache.txt CMakeFiles/

cmake -Werror=dev -Werror=deprecated .

make clean
make
make test
