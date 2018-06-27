#!/bin/bash
Path=$1/svn_version.h
UPDATE_RANDOM=`cat /dev/urandom|sed 's/[^a-zA-Z0-9]//g'|strings -n 6|head -n 1`
SVN_VERSION=`svn info|sed -n '/^Revision/{p}'| awk '{print $2}'`

echo "#ifndef SVN_VERSION_INCLUDE_H " > $Path
echo "#define SVN_VERSION_INCLUDE_H " >> $Path
echo "#define SVN_VERSION \"${SVN_VERSION}\"" >> $Path
echo "#define UPDATE_RANDOM \"${UPDATE_RANDOM}_${SVN_VERSION}\"" >> $Path
echo "#endif" >> $Path

