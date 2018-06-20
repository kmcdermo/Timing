#!/bin/bash

indir=${1}
files=${2}
wgtfile=${3}

root -b -q -l computeSumWeights.C\(\"${indir}\",\"${files}\",\"${wgtfile}\"\)

## Final message
echo "Finished ComputingSumWeights for:" ${files}
