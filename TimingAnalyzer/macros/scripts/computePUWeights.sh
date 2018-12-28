#!/bin/bash

## config
indir=${1}
files=${2}
puwgtfile=${3}

## run macro
root -l -b -q computePUWeights.C\(\"${indir}\",\"${files}\",\"${puwgtfile}\"\)

## Final message
echo "Finished ComputingPUWeights for: " ${files}
