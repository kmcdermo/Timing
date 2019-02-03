#!/bin/bash

## source first
source scripts/common_variables.sh

## config
eraplotconfig=${1:-"etaplotconfig.${inTextExt}"}
plotconfig=${2:-"${plotconfigdir}/met_zoom.${inTextExt}"}
savemetadata=${3:-0}
outfiletext=${4:-"plot_era"}
dir=${5:-"test"}

## first make plot
root -l -b -q runEraPlotter.C\(\"${eraplotconfig}\",\"${plotconfig}\",${savemetadata},\"${outfiletext}\"\)

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
echo "Finished EraPlotting for plot:" ${plotconfig}
