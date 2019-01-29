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
for canvscale in "${canvscales[@]}"
do
    for ext in "${exts[@]}"
    do
	cp "${sig_outtext}_${canvscale}.${ext}" "${fulldir}"
    done
done
cp "${sig_outtext}.root" "${fulldir}"

## Final message
echo "Finished PlottingSignificances"
