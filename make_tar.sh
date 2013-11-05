#!/bin/bash
version=0.03
tar --exclude=data --exclude=doc --exclude=html --exclude=usml_api.html \
    -czf ../usml-src-$version.tar.gz *
tar -czf ../usml-doc-$version.tar.gz usml_api.html html/*
tar -czf ../usml-data-$version.tar.gz data/*
zip --quiet -r ../usml-src-$version.zip * \
    -x "data/*" -x "doc/*" -x "html/*" -x usml_api.html
zip --quiet -r ../usml-doc-$version.zip usml_api.html html/*
zip --quiet -r ../usml-data-$version.zip data/*
cp usml_install.html ..
cp usml_release_notes.html ..
cp doc/usml_frontpage.html ..
cp COPYRIGHT.txt ..
