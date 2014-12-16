#!/bin/bash
# Creates USML tarballs and zip files for delivery to
# the Ocean Acoustic Library http://oalib.hlsresearch.com/.

set -e  # exit on error

# read options from command line

IS_BRIEF=
while [ $# -ge 1 ] ; do
  case "$1" in
    --brief) 
        IS_BRIEF=Y;;
    -h|--help) 
        echo "Builds USML tarballs and zip files for delivery."
        echo "The --brief option limits this to the source tarball."
        exit;;
    *) break;;
  esac
  shift
done

# load version number from disk

DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )
version=$(cat $DIR/VERSION.txt)
cd $DIR/..
OUTPUT=../usml-delivery
mkdir -p $OUTPUT

# create tar ball and zip file for source
# do not follow or archive symbolic links

echo "build usml-src-$version"
find * -type f | tar --exclude=data --exclude=doc --exclude=html --exclude=usml_api.html --exclude=make_tar* \
	--exclude '*.nc' --exclude '*.csv' --exclude '*.grd' --exclude '*.asc' \
    -czf $OUTPUT/usml-src-$version.tar.gz -T -

if [ ! -z "$IS_BRIEF" ]; then
  exit
fi

find * -type f | zip --quiet -r  $OUTPUT/usml-src-$version.zip -@ \
    -x "data/*" -x "doc/*" -x "html/*" -x usml_api.html -x make_tar* -x '*.nc' -x '*.csv' \
    -x '*.grd' -x '*.asc'

# create tarball and zip file for documentation
# create documentation if necessary

echo "build usml-doc-$version"
if [ ! -d html ]; then
    doxygen
fi
tar -czf $OUTPUT/usml-doc-$version.tar.gz usml_api.html html/*
zip --quiet -r $OUTPUT/usml-doc-$version.zip usml_api.html html/*

# create tarball and zip file for data

echo "build usml-data-$version"
tar -czf $OUTPUT/usml-data-$version.tar.gz data/*
zip --quiet -r $OUTPUT/usml-data-$version.zip data/*

# copy files that don't get wrapped in tarballs

cp usml_install.html $OUTPUT
cp usml_release_notes.html $OUTPUT
cp doc/usml_frontpage.html $OUTPUT
cp doc/theory/ReillyGaussianSphericalTime.pdf $OUTPUT
cp doc/test/ReillyTestingGaussianSphericalTime.pdf $OUTPUT
cp COPYRIGHT.txt $OUTPUT

# display completion message

cd $OUTPUT
echo "files can be found in $PWD"
