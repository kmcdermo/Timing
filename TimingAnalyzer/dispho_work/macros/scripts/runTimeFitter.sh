#!/bin/bash

## source first
source scripts/common_variables.sh

## config
infilename=${1:-"${skimdir}/sr.root"}
plotconfig=${2:-"${plotconfigdir}/phopt_0.${inTextExt}"}
timefitconfig=${3:-"${miscconfigdir}/misc_blind.${inTextExt}"}
era={$4:-"full"}
outfiletext=${5:-"plots"}
dir=${6:-"plots/test"}

## first make plot
root -l -b -q runTimeFitter.C\(\"${infilename}\",\"${plotconfig}\",\"${timefitconfig}\",\"${era}\",\"${outfiletext}\"\)

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
echo "Finished TimeFitting for plot:" ${plotconfig}
