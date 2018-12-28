#!/bin/bash

## source first
source scripts/common_variables.sh

## config
indir=${1:-"output"}
infilename=${2:-"AsymLim"}
limitconfig=${3:-"${limitconfigdir}/limits2D.${inTextExt}"}
era=${4:-"Full"}
outtext=${5:-"limit2D"}
dir=${6:-"ntuples_v4/limits"}

## run macro
root -l -b -q runLimits2D.C\(\"${indir}\",\"${infilename}\",\"${limitconfig}\",\"${era}\",\"${outtext}\"\)

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
