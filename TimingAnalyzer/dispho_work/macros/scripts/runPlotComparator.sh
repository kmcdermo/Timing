#!/bin/bash

## source first
source scripts/common_variables.sh

## config
compareconfig=${1:-"plot_compare.${inTextExt}"}
outfiletext=${2:-"plots"}
dir=${3:-"plots/test"}

## first make plot
root -l -b -q runTreePlotter.C\(\"${compareconfig}\",\"${outfiletext}\"\)

## make out dirs
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir ${fulldir}

## copy everything
for canvscale in "${canvscales[@]}"
do
    for ext in "${exts[@]}"
    do
	cp ${outfiletext}_${canvscale}.${ext} ${fulldir}
    done
done
cp ${outfiletext}.root ${fulldir}

## Final message
echo "Finished PlotComparing"
