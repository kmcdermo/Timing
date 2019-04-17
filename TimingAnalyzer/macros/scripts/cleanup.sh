#!/bin/bash

## always source first
source scripts/common_variables.sh

## delete local outputs
rm *".${outTextExt}"
rm *".root"
for ext in "${exts[@]}"
do
    rm *".${ext}"
done

## print out message
echo "Finished cleaning up outputs!"
