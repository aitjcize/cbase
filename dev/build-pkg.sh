#!/bin/bash

program_name=cbase

if [ "$1" == "clean" ]; then
  rm -rf ${program_name}*
  exit 0
fi

echo 'Version: '
read ver

./chver.sh $ver
cd ../../
cp -r $program_name $program_name-$ver
rm -rf $program_name-$ver/dev
mv $program_name-$ver $program_name/dev
cd $program_name/dev/

if [ "$1" == "sdist" ]; then
  tar -zcf ${program_name}-$ver.tar.gz $program_name-$ver
  exit 0
else
  tar -zcf ${program_name}_$ver.orig.tar.gz $program_name-$ver
fi

cd $program_name-$ver
dh_make -s -b -p $program_name

cp ../rules ../control debian
cd debian
rm *.ex *.EX README.*
dch -e
cd ..

if [ "$1" == "deb" ] || [ -z "$1" ]; then
  dpkg-buildpackage -rfakeroot
elif [ "$1" == "ppa" ]; then
  debuild -S
fi
