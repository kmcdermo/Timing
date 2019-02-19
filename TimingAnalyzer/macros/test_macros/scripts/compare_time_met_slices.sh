#!/bin/bash

## source
source scripts/common_variables.sh

## input from command line
outfiletext=${1:-"time_vs_met_slices"}
outdir=${2:-"madv2_v3/checks"}

## global config
infilename="met_vs_time_templates.root"

## run macro
root -l -b -q test_macros/compare_time_met_slices.C\(\"${infilename}\",\"${outfiletext}\"\)

## copy the latest to outdir
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

for ext in "${exts[@]}"
do
    cp "${outfiletext}.${ext}" "${fulldir}"
done
cp "${outfiletext}.root" "${fulldir}"

## end message
echo "All done!"
