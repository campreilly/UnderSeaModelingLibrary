#!/bin/bash
version=0.02
tar --exclude=data --exclude=doc --exclude=html --exclude= usml-api.html \
    -czf ../usml-src-$version.tar.gz *
tar -czf ../usml-doc-$version.tar.gz usml-api.html html/*
zip --quiet -r ../usml-src-$version.zip * \
    -x "data/*" -x "doc/*" -x "html/*" -x usml-api.html
zip --quiet ../usml-doc-$version.zip usml-api.html html/*
