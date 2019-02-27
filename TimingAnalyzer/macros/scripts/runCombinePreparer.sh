#!/bin/bash

## source first
source scripts/common_variables.sh

## config
infilename=${1:-"met_vs_time.root"}
bininfoname=${2:-"bininfo.${inTextExt}"}
ratioinfoname=${3:-"ratioinfo.${inTextExt}"}
binratioinfoname=${4:-"binratioinfo.${inTextExt}"}
systfilename=${5:-"${systconfigdir}/systematics.${inTextExt}"}
datacardname=${6:-"datacardABCD"}
blind_data=${7:-1}
outdir=${8:-"madv2_v3/checks_v23"}

## run macro
root -l -b -q runCombinePreparer.C\(\"${infilename}\",\"${bininfoname}\",\"${ratioinfoname}\",\"${binratioinfoname}\",\"${systfilename}\",\"${datacardname}\",${blind_data}\)

## make outdirs readable
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy files
echo "Copying output to ${fulldir}"
cp "${datacardname}"*".${inTextExt}" "${fulldir}"
cp "${wsfilename}" "${fulldir}"

## Final message
echo "Finished PreparingCombine"
