#!/bin/bash
#VERSION="debug"
VERSION="release"

echo "Cleaning the $VERSION version"
cd ../usml-${VERSION}
make clean
rm -rf libusml.*
rm -rf usml_test
rm -rf cmp_speed
rm -rf eigenray_extra_test
rm -rf malta_movie
rm -rf malta_rays
rm -rf pederson_test
rm -rf ray_speed
rm -rf waveq3d_visual

cd ..
