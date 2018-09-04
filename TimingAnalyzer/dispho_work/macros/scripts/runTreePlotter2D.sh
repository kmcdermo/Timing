#!/bin/bash

## source first
source scripts/common_variables.sh

infilename=${1:-"${skimdir}/sr.root"}
insignalfilename=${2:-"${skimdir}/signals_sr.root"}
cutconfig=${3:-"${cutconfigdir}/signal_blinded.${inTextExt}"}
varwgtmapconfig=${4:-"${varwgtconfigdir}/empty.${inTextExt}"}
plotconfig=${5:-"${plotconfigdir}/met_vs_time.${inTextExt}"}
miscconfig=${6:-"${miscconfigdir}/blind2D.${inTextExt}"}
era=${7:-"Full"}
outfiletext=${8:-"met_vs_time.root"}
dir=${9:-"plots2D"}

## run tree plotter first
root -l -b -q runTreePlotter2D.C\(\"${infilename}\",\"${insignalfilename}\",\"${cutconfig}\",\"${varwgtmapconfig}\",\"${plotconfig}\",\"${miscconfig}\",\"${era}\",\"${outfiletext}\"\)

## make out dirs
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir ${fulldir}

## copy everything
cp ${outfiletext}.root ${fulldir}

## Final message
echo "Finished TreePlotting2D for plot:" ${plotconfig}
