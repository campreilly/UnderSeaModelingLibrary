#!/bin/sh

SPEC_FILE_PATH="../"
SPEC_FILE_NAME="usml.spec"
NAME=usml
VERSION=`cat $SPEC_FILE_PATH$SPEC_FILE_NAME | grep Version: | sed s/^.*": "// | sed 's/\s*$//g'`

if [ -d /var/tmp/$NAME ]; then 
	rm -Rf /var/tmp/$NAME
fi

cd ../..

# Copy only src related items
rsync -r --exclude '.metadata' --exclude '.settings' --exclude 'data' --exclude '.git*' --exclude 'doc' --exclude 'gulf_oman*.*' --exclude 'html' --exclude 'matlab' --exclude '*~' --exclude 'make_*' --copy-links $NAME /var/tmp

#change to library source directory
cd /var/tmp/$NAME

#remove cores
/home/safuser/dev/removeCores.sh

cd /var/tmp

echo "Creating $NAME.tgz" 

tar -czf $NAME-$VERSION.tgz $NAME

echo "Copying $NAME-$VERSION.tgz to ~/rpmbuild/SOURCES" 
cp $NAME-$VERSION.tgz ~/rpmbuild/SOURCES
cp $NAME-$VERSION.tgz ~/rpmbuild/SOURCES/$NAME.tgz 
rm -rf /var/tmp/$NAME

echo "Complete"
