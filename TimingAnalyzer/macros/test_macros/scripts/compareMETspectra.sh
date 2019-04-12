#!/bin/bash

## source first
source scripts/common_variables.sh

## config
indir=${1:-"inclusive_signals"}
outfiletext=${2:-"metspectra"}
outdir=${3:-"madv2_v4/metspectra"}

## first make plot
root -l -b -q test_macros/compareMETspectra.C\(\"${indir}\",\"${outfiletext}\"\)

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy everything (all samples)
for ext in "${exts[@]}"
do
    cp "${outfiletext}_"*".${ext}" "${fulldir}"
done
cp "${outfiletext}.root" "${fulldir}"

## Final message
echo "Finished Comparing MET Spectra"
