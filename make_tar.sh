#!/bin/bash
version=0.02
tar --exclude=data --exclude=doc --exclude=html --exclude=usml_api.html \
    -czf ../usml-src-$version.tar.gz *
tar -czf ../usml-doc-$version.tar.gz usml_api.html html/*
zip --quiet -r ../usml-src-$version.zip * \
    -x "data/*" -x "doc/*" -x "html/*" -x usml_api.html
zip --quiet ../usml-doc-$version.zip usml_api.html html/*
cp usml_install.html ..
