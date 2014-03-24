#!/bin/bash
version=0.05b

# Do not follow or archive symbolic links
find * -type f | tar --exclude=data --exclude=doc --exclude=html --exclude=usml_api.html --exclude=make_tar* \
	 --exclude '*.nc' --exclude '*.csv' --exclude '*.grd' --exclude '*.asc' \
    -czf ../usml-src-$version.tar.gz -T -
tar -czf ../usml-doc-$version.tar.gz usml_api.html html/*
tar -czf ../usml-data-$version.tar.gz data/*

# Do not follow or archive symbolic links
find * -type f | zip --quiet -r  ../usml-src-$version.zip -@ \
    -x "data/*" -x "doc/*" -x "html/*" -x usml_api.html -x make_tar* -x '*.nc' -x '*.csv' \
    -x '*.grd' -x '*.asc'
zip --quiet -r ../usml-doc-$version.zip usml_api.html html/*
zip --quiet -r ../usml-data-$version.zip data/*
cp usml_install.html ..
cp usml_release_notes.html ..
cp doc/usml_frontpage.html ..
cp COPYRIGHT.txt ..
