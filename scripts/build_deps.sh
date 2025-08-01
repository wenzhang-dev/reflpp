#!/bin/bash

function build_fmt() {
    pushd third-party/fmt

    rm -rf build
    echo "build dir: $(pwd)/build"
    mkdir -p build

    cd ./build
    cmake -DCMAKE_INSTALL_PREFIX=`pwd` -DFMT_TEST=OFF ..
    make -j8
    make install

    popd
}

build_fmt
