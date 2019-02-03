#!/bin/bash

## source first
source scripts/common_variables.sh

## config
crtosrconfig=${1:-"${crtosrconfigdir}/qcd_phoseedtime_0.${inTextExt}"}
era=${2:-"Full"}
savemetadata=${3:-0}
outfiletext=${4:-"qcd_phoseedtime_0"}
dir=${5:-"ntuples_v4/checks_v3/CRtoSR"}

## first make plot
root -l -b -q runCRtoSRPlotter.C\(\"${crtosrconfig}\",\"${era}\",${savemetadata},\"${outfiletext}\"\)

## make out dirs
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir ${fulldir}

## copy everything
cp ${outfiletext}.root ${fulldir}
for plotscale in norm scaled
do
    for canvscale in "${canvscales[@]}"
    do
	for ext in "${exts[@]}"
	do
	    cp ${outfiletext}_${plotscale}_${canvscale}.${ext} ${fulldir}
	done
    done
done

## Final message
echo "Finished CRtoSRPlotting for:" ${crtosrconfig}
