#!/bin/bash

## source first
source scripts/common_variables.sh

## config
infilename=${1:-"${skimdir}/sr.root"}
outfiletext=${2:-"plots"}
dir=${3:-"test"}

## first make plot
root -l -b -q runTnPPlotter.C\(\"${infilename}\",\"${outfiletext}\"\)

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
