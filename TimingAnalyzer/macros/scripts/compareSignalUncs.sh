#!/bin/bash

## source first
source scripts/common_variables.sh

## config
nomfilename=${1:-"${skimdir}/signals_sr_nom.root"}
uncfilename=${2:-"${skimdir/}signals_sr_unc.root"}
systuncname=${3:-"phoscaleup"}
systunclabel=${4:-"Photon_Scale_Up"}
outdir=${5:-"test"}

## replace _ with " "
systunclabel=$( echo "${systunclabel}" | tr "_" " ")

## first make plots
root -l -b -q compareSignalUncs.C\(\"${nomfilename}\",\"${uncfilename}\",\"${syst_unc_name}\",\"${syst_unc_label}\"\)

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy everything
for ext in "${exts[@]}"
do
    cp "${systuncname}_*.${ext}" "${fulldir}"
done
cp "${systuncname}_plots.root" "${fulldir}"

## Final message
echo "Finished Comparing Singal 1D Uncs"
