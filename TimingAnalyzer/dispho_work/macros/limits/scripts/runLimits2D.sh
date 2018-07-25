#!/bin/bash

## source first
source scripts/common_variables.sh

## config
indir=${1:-"output"}
infilename=${2:-"AsymLim"}
limitconfig=${3:-"${limitconfigdir}/limits2D.${inTextExt}"}
outtext=${4:-"limit2D"}
dir=${5:-"plots/ntuples_v4/limits"}

## run macro
root -l -b -q runLimits2D.C\(\"${indir}\",\"${infilename}\",\"${limitconfig}\",\"${outtext}\"\)

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
