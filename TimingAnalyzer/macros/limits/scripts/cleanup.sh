#!/bin/bash

## source first
source scripts/common_variables.sh

## delete local compiled objects
rm *.so
rm *.d
rm *.pcm

## delete local outputs
rm *.${outTextExt}
rm *.root
for ext in "${exts[@]}"
do
    rm *.${ext}
done

## clean-up combine info
rm "${combdir}/"*".${inTextExt}" "${combdir}/"*".root"

## print out message
this_dir=$( pwd )
echo "Finished cleaning up ${this_dir}"
