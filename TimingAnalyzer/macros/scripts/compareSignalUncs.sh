#!/bin/bash

## source first
source scripts/common_variables.sh

## config
scan_log=${1:-"abcd_categories.log"}
nomfilename=${2:-"${skimdir}/signals_sr_nom.root"}
uncfilename=${3:-"${skimdir}/signals_sr_unc.root"}
systuncname=${4:-"phoscaleup"}
systunclabel=${5:-"Photon_Scale_Up"}
outdir=${6:-"test"}
extra_shift=${7:-"0"} # ns for time unc tests

## replace _ with " "
systunclabel=$( echo "${systunclabel}" | tr "_" " ")

## first make plots
root -l -b -q compareSignalUncs.C\(\"${scan_log}\",\"${nomfilename}\",\"${uncfilename}\",\"${systuncname}\",\""${systunclabel}"\",\"${extra_shift}\"\)

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy everything
for ext in "${exts[@]}"
do
    cp "${systuncname}_"*".${ext}" "${fulldir}"
done
cp "${systuncname}_plots.root" "${fulldir}"

## Final message
echo "Finished Comparing Singal 1D Uncs"
