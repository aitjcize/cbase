#!/bin/bash

sed -i "s/AC_INIT(\[cbase\], \[.*\], \[aitjcize@gmail.com\])/AC_INIT([cbase], [$1], [aitjcize@gmail.com])/g" ../configure.ac
