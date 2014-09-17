#!/bin/bash

export VERSION=0.05.5
export NAME=usml

if [ -d /var/tmp/$NAME ]; then 
	rm -Rf /var/tmp/$NAME
fi

cd ../..

# Copy only src related items
rsync -r --exclude '.metadata' --exclude '.settings' --exclude 'data' --exclude '.git*' --exclude 'doc' --exclude 'matlab' --exclude '*~' --exclude 'make_*' --copy-links $NAME /var/tmp

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

echo "Creating $NAME.tgz" 

tar -czf $NAME.tgz $NAME

echo "Copying $NAME-$VERSION.tgz to ~/rpmbuild/SOURCES" 
cp $NAME.tgz ~/rpmbuild/SOURCES
cp ~/rpmbuild/SOURCES/$NAME.tgz ~/rpmbuild/SOURCES/$NAME-$VERSION.tgz
rm -rf /var/tmp/$NAME
rm $NAME.tgz

echo "Complete"
