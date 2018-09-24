#!/bin/bash

## config
indir=${1}
outdir=${2}
filename=${3}
sumwgts=${4}
skimtype=${5:-"Standard"}
puwgtfilename=${6:-""}

## run macro
root -b -q -l runSkimmer.C\(\"${indir}\",\"${outdir}\",\"${filename}\",${sumwgts},\"${skimtype}\",\"${puwgtfilename}\"\)

## Final message
echo "Finished Skimming for file:" ${filename}
