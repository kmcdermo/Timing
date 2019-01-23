#!/bin/bash

## config
iofilename=${1:-"met_vs_time.root"}
xbin_boundary=${2:-"3"}
ybin_boundary=${3:-"200"}
blind_data=${4:-1}
signif_dump=${5:-"tmp_dump.txt"}

## run macro
root -l -b -q dumpSignificanceABCD.C\(\"${iofilename}\",\"${xbin_boundary}\",\"${ybin_boundary}\",${blind_data},\"${signif_dump}\"\)

## Final message
echo "Finished DumpingSignificanceABCD for: ${iofilename}" 
