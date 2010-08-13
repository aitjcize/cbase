#!/bin/bash

program_name=cbase

echo 'Version: '
read ver

./chver.sh $ver
cd ../../
cp -r $program_name $program_name-$ver
mv $program_name-$ver $program_name/dev
cd $program_name/dev/
tar -zcf ${program_name}_$ver.orig.tar.gz $program_name-$ver

cd $program_name-$ver
dh_make -s -b -p $program_name

echo `pwd`
cp dev/rules dev/control dev/manpages debian
cd debian
rm *.ex *.EX README.*
dch -e
cd ..

if [ "$1" == "deb" ]; then
  dpkg-buildpackage -rfakeroot
elif [ "$1" == "ppa" ]; then
  debuild -S
fi
