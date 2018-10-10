#!/bin/bash

## source first
source scripts/common_variables.sh

## config
skimfilename=${1:-"${skimdir}/sr.root"}
signalskimfilename=${2:-"${skimdir}/signals_sr.root"}
infilesconfig=${3:-"files.${inTextExt}"}
stime=${4:-"seedtime"}
doshift=${5:-0}
dosmear=${6:-0}

## first make plot
root -l -b -q runTimeAdjuster.C\(\"${skimfilename}\",\"${signalskimfilename}\",\"${infilesconfig}\",\"${stime}\",${doshift},${dosmear}\)

## Final message
echo "Finished TimeAdjusting for files: ${skimfilename} and ${signalskimfilename}"
