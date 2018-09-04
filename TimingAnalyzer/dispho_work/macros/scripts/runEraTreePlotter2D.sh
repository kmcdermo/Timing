#!/bin/bash

## source first
source scripts/common_variables.sh

## config
infilename=${1:-"${skimdir}/sr.root"}
insignalfilename=${2:-"${skimdir}signals_sr.root"}
cutconfig=${3:-"${cutconfigdir}/always_true.${inTextExt}"}
varwgtmapconfig=${4:-"${varwgtconfigdir}/empty.${inTextExt}"}
plotconfig=${5:-"${plotconfigdir}/met_vs_time.${inTextExt}"}
miscconfig=${6:-"${miscconfigdir}/misc_blind.${inTextExt}"}
outfiletext=${7:-"plots"}
era=${8-"2017F"}
dir=${9:-"plots/test"}

## first make plot
root -l -b -q runEraTreePlotter2D.C\(\"${infilename}\",\"${insignalfilename}\",\"${cutconfig}\",\"${varwgtmapconfig}\",\"${plotconfig}\",\"${miscconfig}\",\"${era}\",\"${outfiletext}\"\)

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
echo "Finished EraTreePlotting2D for plot:" ${plotconfig}
