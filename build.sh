#!/bin/bash

# Args: $1=BUILD_DIR, $2=BUILD_TYPE
BUILD_DIR=${1:-build}
BUILD_TYPE=${2:-RELEASE}

if [ -d "./$BUILD_DIR" ]; then
  rm -rf ./$BUILD_DIR
fi

mkdir $BUILD_DIR

cd $BUILD_DIR

if [ "$BUILD_TYPE" == "DEBUG" ]; then
    CMAKE_ARGS="-DCMAKE_BUILD_TYPE=DEBUG"
elif [ "$BUILD_TYPE" == "RELEASE_TESTS" ]; then
    CMAKE_ARGS="-Denable_tests=ON -DCMAKE_BUILD_TYPE=RELEASE"
else
    CMAKE_ARGS="-DCMAKE_BUILD_TYPE=RELEASE"
fi

if [ "$INSTALL_DEPS" == "ON" ]; then
    CMAKE_ARGS="$CMAKE_ARGS -DINSTALL_DEPS=ON"
fi

echo "CMake Args: $CMAKE_ARGS"
cmake .. $CMAKE_ARGS
if [ $? -ne 0 ]; then
  echo "CMake failed"
  exit 1
fi

make
if [ $? -ne 0 ]; then
  echo "Build failed"
  exit 1
fi