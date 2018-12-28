#!/bin/bash

## always source first
source scripts/common_variables.sh

## recursively remove all tmp files
find . -name "*~" -type f -delete
find . -name "*#" -type f -delete

## delete local compiled objects
rm *.so
rm *.d
rm *.pcm

## delete local outputs
rm *.${outTextExt}
rm *.root
rm *.png
rm *.pdf
rm *.eps

## print out message
this_dir=$( pwd )
echo "Finished cleaning up ${this_dir}"
