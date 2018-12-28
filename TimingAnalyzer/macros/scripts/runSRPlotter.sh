#!/bin/bash

## source first
source scripts/common_variables.sh

## config
srplotconfig=${1:-"${srplotconfigdir}/phoseedtime_0.${inTextExt}"}
miscconfig=${2:-"${miscconfigdir}/misc_blind.${inTextExt}"}
era=${3:-"Full"}
outfiletext=${4:-"phoseedtime_0_SRPlot"}
dir=${5:-"ntuples_v4/checks_v3/kF_v2"}

## first make plot
root -l -b -q runSRPlotter.C\(\"${srplotconfig}\",\"${miscconfig}\",\"${era}\",\"${outfiletext}\"\)

## make out dirs
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir ${fulldir}

## copy everything
for label in "" "_CR_GJets_kFScaled" "_CR_QCD_kFScaled"
do
    for canvscale in "${canvscales[@]}"
    do
	for ext in "${exts[@]}"
	do
	    cp ${outfiletext}${label}_${canvscale}.${ext} ${fulldir}
	done
    done
    
    cp ${outfiletext}${label}.root ${outfiletext}${label}_integrals.${outTextExt} ${fulldir}
done

## Final message
echo "Finished SRPlotting for:" ${srplotconfig}
