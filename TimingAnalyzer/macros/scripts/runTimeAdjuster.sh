#!/bin/bash

## source first
source scripts/common_variables.sh

## config
skimfilename=${1:-"${skimdir}/sr.root"}
signalskimfilename=${2:-"${skimdir}/signals_sr.root"}
infilesconfig=${3:-"files.${inTextExt}"}
sadjustvar=${4:-"phoE"}
stime=${5:-"seedtime"}
doshift=${6:-0}
dosmear=${7:-0}
skipdata=${8:-0}
skipbkgdmc=${9:-0}
savemetadata=${10:-0}

## first make plot
root -l -b -q runTimeAdjuster.C\(\"${skimfilename}\",\"${signalskimfilename}\",\"${infilesconfig}\",\"${sadjustvar}\",\"${stime}\",${doshift},${dosmear},${skipdata},${skipbkgdmc},${savemetadata}\)

## Final message
echo "Finished TimeAdjusting for files: ${skimfilename} and ${signalskimfilename}"
