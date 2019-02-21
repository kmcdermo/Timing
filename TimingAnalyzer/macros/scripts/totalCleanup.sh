#!/bin/bash

## always source first
source scripts/common_variables.sh

## delete outputs first
./scripts/cleanup.sh

## recursively remove all tmp files
find . -name "*~" -type f -delete
find . -name "*#" -type f -delete

## delete local compiled objects
rm *.so
rm *.d
rm *.pcm

## print out message
echo "Finished cleaning up it all!"
