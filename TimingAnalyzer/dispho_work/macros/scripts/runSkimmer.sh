#!/bin/bash

indir=${1}
outdir=${2}
filename=${3}
sumwgts=${4}
puwgtfilename=${5:-""}
redophoid=${6:-0}

root -b -q -l runSkimmer.C\(\"${indir}\",\"${outdir}\",\"${filename}\",${sumwgts},\"${puwgtfilename}\",${redophoid}\)

## Final message
echo "Finished Skimming for file:" ${filename}
