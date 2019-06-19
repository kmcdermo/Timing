#!/bin/bash

## source first
source scripts/common_variables.sh

## config
leftname=${1:-"left.root"}
rightname=${2:-"right.root"}
cut_label=${3:-"var"}
split=${4:-"1"}
cut_unit=${5:-"unit"}
signal=${6:-"GMSB_L200_CTau200"}
finalname=${7:-"final"}
outdir=${8:-"madv2_v4p1/data_v_data"}

## first make plot
root -l -b -q drawDataVData.C\(\"${leftname}\",\"${rightname}\",\"${cut_label}\",\"${split}\",\"${cut_unit}\",\"${signal}\",\"${finalname}\"\)

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy everything
declare -a scales=("absnorm" "scaled")
for scale in "${scales[@]}"
do
    for canvscale in "${canvscales[@]}"
    do
	for ext in "${exts[@]}"
	do
	    cp "${finalname}_${scale}_${canvscale}.${ext}" "${fulldir}"
	done
    done
done
cp "${finalname}.root" "${fulldir}"

## Final message
echo "Finished drawing data vs data for plot: ${finalname}"
