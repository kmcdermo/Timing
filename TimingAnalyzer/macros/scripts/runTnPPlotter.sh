#!/bin/bash

## source first
source scripts/common_variables.sh

## config
infilename=${1:-"${skimdir}/sr.root"}
savemetadata=${2:-0}
outfiletext=${3:-"plots"}
dir=${4:-"test"}

## first make plot
root -l -b -q runTnPPlotter.C\(\"${infilename}\",${savemetadata},\"${outfiletext}\"\)

## make out dirs
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir ${fulldir}

## copy everything
for eta in "${etas[@]}"
do
    for canvscale in "${canvscales[@]}"
    do
	for ext in "${exts[@]}"
	do
	    cp ${outfiletext}_${eta}_${canvscale}.${ext} ${fulldir}
	done
    done
done
cp ${outfiletext}.root ${fulldir}

## Final message
echo "Finished TnPPlotting..."
