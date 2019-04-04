#!/bin/bash

## source first
source scripts/common_variables.sh

## config
label=${1:-"QCD"}
outfiletext=${2:-"met_vs_time"}
outdir=${3:-"madv2_v4/uncs/closure"}

## run extractor
root -l -b -q extractClosureUncertainty.C\(\"${label}\",\"${outfiletext}\"\)

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy everything
cp "${outfiletext}_${label}.root" "${fulldir}"

## Final message
echo "Finished Extracting Closure Uncertainty for: ${label}"
