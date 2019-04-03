#!/bin/bash

## source first
source scripts/common_variables.sh

## config
scan_log=${1:-"abcd_categories.log"}
systuncname=${2:-"phoscaleup"}
outdir=${3:-"madv2_v4/uncs/phoscaleup/diffs"}

## first make plots
root -l -b -q extractSigDiffs.C\(\"${scan_log}\",\"${plotfiletext}\",\"${nom2Ddir}\",\"${unc2Ddir}\",\"${systuncname}\"\)

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy text files
cp "${systuncname}_*.${outTextExt}" "${fulldir}"

## Final message
echo "Finished Comparing Diffs in ABCD regions"
