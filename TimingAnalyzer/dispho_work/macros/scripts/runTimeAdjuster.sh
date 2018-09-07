#!/bin/bash

## source first
source scripts/common_variables.sh

## config
skimfilename=${1:-"${skimdir}/sr.root"}
signalskimfilename=${2:-"${skimdir}/signals_sr.root"}
infilesconfig=${3:-"files.${inTextExt}"}
doshift=${4:-0}
dosmear=${5:-0}

## first make plot
root -l -b -q runTimeAdjuster.C\(\"${skimfilename}\",\"${signalskimfilename}\",\"${infilesconfig}\",${doshift},${dosmear}\)

## Final message
echo "Finished TimeAdjusting for files: ${skimfilename} and ${signalskimfilename}"
