#!/bin/bash

## source first
source scripts/common_variables.sh

## config
eraplotconfig=${1:-"etaplotconfig.${inTextExt}"}
plotconfig=${2:-"${plotconfigdir}/met_zoom.${inTextExt}"}
outfiletext=${3:-"plot_era"}
dir=${4:-"test"}

## first make plot
root -l -b -q runEraPlotter.C\(\"${eraplotconfig}\",\"${plotconfig}\",\"${outfiletext}\"\)

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
