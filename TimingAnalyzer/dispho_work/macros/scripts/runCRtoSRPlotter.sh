#!/bin/bash

## source first
source scripts/common_variables.sh

## config
crtosrconfig=${1:-"${crtosrconfigdir}/qcd_phoseedtime_0.${inTextExt}"}
outfiletext=${2:-"qcd_phoseedtime_0"}
dir=${3:-"plots/ntuples_v4/checks_v3/CRtoSR"}

## first make plot
root -l -b -q runCRtoSRPlotter.C\(\"${crtosrconfig}\",\"${outfiletext}\"\)

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
