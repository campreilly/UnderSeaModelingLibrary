#!/bin/bash
#VERSION="debug"
VERSION="release"

echo "Building the $VERSION version"
#Begin compiling usml files
cd ../../usml-${VERSION}
make all

cd ..

