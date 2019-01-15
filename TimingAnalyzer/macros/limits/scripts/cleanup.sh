#!/bin/bash

## source first
source scripts/common_variables.sh

## delete local compiled objects
rm *.so
rm *.d
rm *.pcm

## delete local outputs
rm *.root
rm *.png
rm *.pdf
rm *.eps
rm *.txt

## clean-up combine info
rm ${combdir}/*.txt ${combdir}/*.root

## print out message
this_dir=$( pwd )
echo "Finished cleaning up ${this_dir}"