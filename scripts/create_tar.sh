#!/bin/sh

SPEC_FILE_PATH="../"
SPEC_FILE_NAME="usml.spec"
NAME=`cat $SPEC_FILE_PATH$SPEC_FILE_NAME | grep Name: | sed s/^.*": "// | sed 's/\s*$//g'`
VERSION=`cat $SPEC_FILE_PATH$SPEC_FILE_NAME | grep Version: | sed s/^.*": "// | sed 's/\s*$//g'`

if [ -d /var/tmp/$NAME ]; then 
	rm -Rf /var/tmp/$NAME
fi

cd ../..

# Copy only src related items
rsync -r --exclude '.metadata' --exclude '.settings' --exclude 'data' --exclude '.git*' --exclude 'doc' --exclude 'html' --exclude 'matlab' --exclude '*~' --exclude 'make_*' --copy-links $NAME /var/tmp

# Copy usml-release dir also
rsync -r --exclude '.metadata' --exclude '.settings' --exclude 'data' --exclude '.git*' --exclude 'doc' --exclude 'matlab' --exclude '*~' --copy-links $NAME-release /var/tmp/$NAME

#change to library source directory
cd /var/tmp/$NAME

# Remove previous builds
cd scripts
./clean.sh
cd ..

#remove cores
/home/safuser/dev/removeCores.sh

cd /var/tmp

echo "Creating $NAME-$VERSION.tgz" 

tar -czf $NAME-$VERSION.tgz $NAME

echo "Copying $NAME-$VERSION.tgz to ~/rpmbuild/SOURCES" 
cp $NAME-$VERSION.tgz ~/rpmbuild/SOURCES
rm -rf /var/tmp/$NAME

echo "Complete"
