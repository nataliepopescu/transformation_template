#!/bin/bash -e

export CC=clang
export CXX=clang++

rm -rf build/ ; 
mkdir build ; 
cd build ; 
cmake -DLLVM_TEMPORARILY_ALLOW_OLD_TOOLCHAIN=true -DCMAKE_INSTALL_PREFIX="./install" -DCMAKE_BUILD_TYPE=Debug ../ ; 
make ;
make install ;
cd ../ 

if ! test -e compile_commands.json ; then
  ln -s build/compile_commands.json ./ ;
fi
