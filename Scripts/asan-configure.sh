#!/usr/bin/env sh

set -eu

CFLAGS="-fsanitize=address -g" CXXFLAGS="-fsanitize=address -g" ./configure

