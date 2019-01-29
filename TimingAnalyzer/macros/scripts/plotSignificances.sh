#!/bin/bash

## source first
source scripts/common_variables.sh

## config
signif_list=${1:-"tmp_list.txt"}
signif_config=${2:-"tmp_signif_config.txt"}
signif_outtext=${3:-"significances"}
dir=${4:-"madv2_v3/full_chain/SR_Blinded/ABCD/inputs"}

## run macro
root -l -b -q plotSignificances.C\(\"${signif_list}\",\"${signif_config}\",\"${signif_outtext}\"\)

## make out dirs
fulldir="${topdir}/${disphodir}/${dir}"
PrepOutDir "${fulldir}"

## cp all to outdir
for ext in "${exts[@]}"
do
    for canvscale in "${canvscales[@]}"
    do
	cp "${signif_outtext}_${canvscale}.${ext}" "${fulldir}"
	cp "${signif_outtext}_BoundariesX_${canvscale}.${ext}" "${fulldir}"
	cp "${signif_outtext}_BoundariesY_${canvscale}.${ext}" "${fulldir}"
    done
    cp "${signif_outtext}_Boundaries2D.${ext}" "${fulldir}"
done
cp "${signif_outtext}.root" "${fulldir}"

## Final message
echo "Finished PlottingSignificances"
