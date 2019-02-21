#!/bin/bash

## source first
source scripts/common_variables.sh

## config
infilename=${1:-"${skimdir}/sr.root"}
insignalfilename=${2:-"${skimdir}signals_sr.root"}
cutconfig=${3:-"${cutconfigdir}/always_true.${inTextExt}"}
plotconfig=${4:-"${plotconfigdir}/phopt_0.${inTextExt}"}
miscconfig=${5:-"${miscconfigdir}/misc_blind.${inTextExt}"}
era=${6:-"Full"}
savemetadata=${7:-0}
outfiletext=${8:-"plots"}
outdir=${9:-"test"}

## first make plot
root -l -b -q runTreePlotter.C\(\"${infilename}\",\"${insignalfilename}\",\"${cutconfig}\",\"${plotconfig}\",\"${miscconfig}\",\"${era}\",${savemetadata},\"${outfiletext}\"\)

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy everything
for canvscale in "${canvscales[@]}"
do
    for ext in "${exts[@]}"
    do
	cp "${outfiletext}_${canvscale}.${ext}" "${fulldir}"
    done
done
cp "${outfiletext}.root" "${outfiletext}_integrals.${outTextExt}" "${fulldir}"

## Final message
echo "Finished TreePlotting for plot: ${plotconfig}"
