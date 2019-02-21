#!/bin/bash

## source first
source scripts/common_variables.sh

## config
infilename=${1:-"met_vs_time.root"}
bininfoname=${2:-"bininfo.${inTextExt}"}
ratioinfoname=${3:-"ratioinfo.${inTextExt}"}
binratioinfoname=${4:-"binratioinfo.${inTextExt}"}
outdir=${5:-"madv2_v3/checks_v13"}

## run macro
root -l -b -q runABCDGenerator.C\(\"${infilename}\",\"${bininfoname}\",\"${ratioinfoname}\",\"${binratioinfoname}\"\)

## make outdirs readable
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy configs
cp "${bininfoname}" "${ratioinfoname}" "${binratioinfoname}" "${fulldir}"

## Final message
echo "Finished GeneratingABCD"
