#!/bin/sh

###############################################
#  Shell script to create usml
#   source and binary rpm file.
###############################################

SCRIPT_FILE_PATH="./"
SPEC_FILE_PATH="../"
SPEC_FILE_NAME="usml.spec"
RPM_NAME=`cat $SPEC_FILE_PATH$SPEC_FILE_NAME | grep Name: | sed s/^.*'\+ '//`
SOURCE_RPM_VERSION=`cat $SPEC_FILE_PATH$SPEC_FILE_NAME | grep Version: | sed s/^.*": "//`
SOURCE_RPM_RELEASE=`cat $SPEC_FILE_PATH$SPEC_FILE_NAME | grep Release: | sed s/^.*": "// | sed s/%{?dist}//`
SOURCE_RPM_PATH="~/rpmbuild/SRPMS/"
SOURCE_RPM_NAME="$RPM_NAME-$SOURCE_RPM_VERSION-$SOURCE_RPM_RELEASE.el6.src.rpm"

###############################################
#  Create the tar file
###############################################
cd $SCRIPT_FILE_PATH
echo
echo "****************************************************************"
echo "Creating tgz file."
echo "****************************************************************"
echo
./create_tar.sh

###############################################
#  Build the source rpm
###############################################
echo 
echo "****************************************************************"
echo "Build the source rpm: $SPEC_FILE_NAME"
echo "****************************************************************"
echo
#remove the old src rpm if it exists
if [ -e $SOURCE_RPM_PATH$SOURCE_RPM_NAME ]; then
  rm $SOURCE_RPM_PATH$SOURCE_RPM_NAME
fi
rpmbuild -bs $SPEC_FILE_PATH$SPEC_FILE_NAME

###############################################
#  Install the source rpm
###############################################
#echo 
#echo "****************************************************************"
#echo "Installing source rpm: $SOURCE_RPM_PATH$SOURCE_RPM_NAME"
#echo "****************************************************************"
#echo
#rpm -ivh $SOURCE_RPM_PATH$SOURCE_RPM_NAME


###############################################
#  Build the binary rpm.
###############################################
echo 
echo "****************************************************************"
echo "Building the binary rpm: $SPEC_FILE_NAME"
echo "****************************************************************"
echo
rpmbuild -bb $SPEC_FILE_PATH$SPEC_FILE_NAME
