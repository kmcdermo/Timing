#!/bin/bash

## source first
source scripts/common_variables.sh

## config
indir=${1:-"output"}
infilename=${2:-"AsymLim"}
doobs=${3:-0}
limitconfig=${4:-"${limitconfigdir}/limits2D.${inTextExt}"}
era=${5:-"Full"}
outtext=${6:-"limit2D"}
dir=${7:-"ntuples_v4/limits"}

## run macro
root -l -b -q runLimits2D.C\(\"${indir}\",\"${infilename}\",${doobs},\"${limitconfig}\",\"${era}\",\"${outtext}\"\)

## make outdirs readable
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir ${fulldir}

## copy everything
for ext in "${exts[@]}"
do
    cp ${outtext}*.${ext} ${fulldir}
done
cp ${outtext}.root ${fulldir}

## Final message
echo "Finished RunningLimits2D"
