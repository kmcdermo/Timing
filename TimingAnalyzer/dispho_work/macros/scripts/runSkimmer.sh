#!/bin/bash

## config
indir=${1}
outdir=${2}
filename=${3}
skimconfig=${4}

## run macro
root -b -q -l runSkimmer.C\(\"${indir}\",\"${outdir}\",\"${filename}\",\"${skimconfig}\"\)

## Final message
echo "Finished Skimming for file:" ${filename}
