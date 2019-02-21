#!/bin/bash

## source first
source scripts/common_variables.sh

infilename=${1:-"${skimdir}/sr.root"}
insignalfilename=${2:-"${skimdir}/signals_sr.root"}
cutconfig=${3:-"${cutconfigdir}/signal_blinded.${inTextExt}"}
plotconfig=${4:-"${plotconfigdir}/met_vs_time.${inTextExt}"}
miscconfig=${5:-"${miscconfigdir}/blind2D.${inTextExt}"}
era=${6:-"Full"}
save_meta_data=${7:-0}
outfiletext=${8:-"met_vs_time.root"}
outdir=${9:-"plots2D"}

## run tree plotter first
root -l -b -q runTreePlotter2D.C\(\"${infilename}\",\"${insignalfilename}\",\"${cutconfig}\",\"${plotconfig}\",\"${miscconfig}\",\"${era}\",${save_meta_data},\"${outfiletext}\"\)

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy everything
cp "${outfiletext}.root" "${fulldir}"

## Final message
echo "Finished TreePlotting2D for plot: ${plotconfig}"
