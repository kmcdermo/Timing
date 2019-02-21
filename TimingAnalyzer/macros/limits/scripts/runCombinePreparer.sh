#!/bin/bash

## source first
source scripts/common_variables.sh

## config
infilename=${1:-"met_vs_time.root"}
bininfoname=${2:-"bininfo.${inTextExt}"}
ratioinfoname=${3:-"ratioinfo.${inTextExt}"}
binratioinfoname=${4:-"binratioinfo.${inTextExt}"}
systfilename=${5:-"${systconfig}/systematics.${inTextExt}"}
wsname=${6:-"workspace"}
datacardname=${7:-"datacardABCD"}
blind_data=${8:-1}
save_meta_data=${9:-0}
wsfilename=${10:-"ws_final.root"}
outdir=${11:-"madv2_v3/checks_v13"}

## run macro
root -l -b -q runCombinePreparer.C\(\"${infilename}\",\"${bininfoname}\",\"${ratioinfoname}\",\"${binratioinfoname}\",\"${systfilename}\",\"${wsname}\",\"${datacardname}\",${blind_data},${save_meta_data}\"${wsfilename}\"\)

## make outdirs readable
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy files
echo "Copying output to ${fulldir}"
cp "${datacardname}"*".${inTextExt}" "${fulldir}"
cp "${wsfilename}" "${fulldir}"

## Final message
echo "Finished PreparingCombine"
