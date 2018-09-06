#!/bin/bash

## source first
source scripts/common_variables.sh

## config
skimfilename=${1:-"${skimdir}/sr.root"}
signalskimfilename=${2:"${skimdir}/signals_sr.root"}
infilesconfig=${3:-"files.${inTextExt}"}
dir=${4:-"plots/test"}

## first make plot
root -l -b -q runTimeAdjuster.C\(\"${skimfilename}\",\"${signalskimfilename}\",\"${infilesconfig}\"\)

## make out dirs
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir ${fulldir}

## Final message
echo "Finished TimeAdjusting for files:" ${skimfilename} "and" ${signalskimfilename}
