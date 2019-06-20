#!/bin/bash

## source variables
source scripts/common_variables.sh

## config
outdir=${1:-"madv2_v4p1/time_style_plots/v3"}
declare -a outnames=("dixtal_inclusive" "dixtal_sameTT" "dixtal_diffTT" "zee")
declare -a plotnames=("datamc" "dataonly")

## run macro
echo "Running macro"
root -l -b -q test_macros/extra_fits/style_time_plots.C

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy everything
for outname in "${outnames[@]}"
do
    for plotname in "${plotnames[@]}"
    do
	outfiletext="${plotname}_${outname}"

	for ext in "${exts[@]}"
	do
	    cp "${outfiletext}.${ext}" "${fulldir}"
	done
	cp "${outfiletext}.C" "${fulldir}"
    done
done
	
## Final message
echo "Finished this"
