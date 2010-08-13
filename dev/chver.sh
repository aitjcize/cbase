#!/bin/bash

sed -i "s/program_version = \".*\"/program_version = \"$1\"/" ../src/cbase.c
