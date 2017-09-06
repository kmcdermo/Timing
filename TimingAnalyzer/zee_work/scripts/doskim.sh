#!/bin/sh

sample=$1

echo "Doing sample: " ${sample}
root -l -b -q "macros/skim.C("\"${sample}\"")"
echo "" 

