#!/bin/bash

cmake -B build -S .
cd build
make clean
make
ctest --output-on-failure
cd ..
rm -rf *.plist
echo "./bin/leds -d 3 -l test"
./bin/leds -d 3 -l test_leds.dat
