#!/bin/sh

###############################################
#  Shell script to create usml
#   devel and binary rpm file.
###############################################

SCRIPT_FILE_PATH="./"
SPEC_FILE_PATH="../"
SPEC_FILE_NAME="usml.spec"
SPEC_DEVEL_FILE_NAME="usml-devel.spec"
RPM_NAME=`cat $SPEC_FILE_PATH$SPEC_FILE_NAME | grep Name: | sed s/^.*": "// | sed 's/\s*$//g'`
DEVEL_RPM_VERSION=`cat $SPEC_FILE_PATH$SPEC_DEVEL_FILE_NAME | grep Version: | sed s/^.*": "// | sed 's/\s*$//g'`
DEVEL_RPM_RELEASE=`cat $SPEC_FILE_PATH$SPEC_DEVEL_FILE_NAME | grep Release: | sed s/^.*": "// | sed s/%{?dist}// | sed 's/\s*$//g'`
DEVEL_RPM_PATH="~/rpmbuild/SRPMS/"
DEVEL_RPM_NAME="$RPM_NAME-$DEVEL_RPM_VERSION-$DEVEL_RPM_RELEASE.el6.devel.rpm"

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
#  Build the devel rpm
###############################################
echo 
echo "****************************************************************"
echo "Build the devel rpm: $SPEC_DEVEL_FILE_NAME"
echo "****************************************************************"
echo
#remove the old devel rpm if it exists
if [ -e $DEVEL_RPM_PATH$DEVEL_RPM_NAME ]; then
  rm $DEVEL_RPM_PATH$DEVEL_RPM_NAME
fi
rpmbuild -bb $SPEC_FILE_PATH$SPEC_DEVEL_FILE_NAME

###############################################
#  Install the devel rpm
###############################################
#echo 
#echo "****************************************************************"
#echo "Installing devel rpm: $DEVEL_RPM_PATH$DEVEL_RPM_NAME"
#echo "****************************************************************"
#echo
#rpm -ivh $DEVEL_RPM_PATH$DEVEL_RPM_NAME


###############################################
#  Build the binary rpm.
###############################################
echo 
echo "****************************************************************"
echo "Building the binary rpm: $SPEC_FILE_NAME"
echo "****************************************************************"
echo
rpmbuild -bb $SPEC_FILE_PATH$SPEC_FILE_NAME
