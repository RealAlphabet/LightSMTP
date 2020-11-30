#!/bin/sh

wget https://github.com/mongodb/mongo-c-driver/releases/download/1.17.2/mongo-c-driver-1.17.2.tar.gz
tar xzf mongo-c-driver-1.17.2.tar.gz
cd mongo-c-driver-1.17.2
mkdir cmake-build
cd cmake-build
cmake -DENABLE_AUTOMATIC_INIT_AND_CLEANUP=OFF ..
cmake --build . --target install
