#!/bin/bash

## source first
source scripts/common_variables.sh

## config
compareconfig=${1:-"plot_compare.${inTextExt}"}
era=${2:-"Full"}
savemetadata=${3:-0}
outfiletext=${4:-"plots"}
dir=${5:-"test"}

## first make plot
root -l -b -q runPlotComparator.C\(\"${compareconfig}\",\"${era}\",${savemetadata},\"${outfiletext}\"\)

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
