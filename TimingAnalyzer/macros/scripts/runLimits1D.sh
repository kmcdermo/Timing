#!/bin/bash

## source first
source scripts/common_variables.sh

## config
indir=${1:-"output"}
infilename=${2:-"AsymLim"}
doobserved=${3:-0}
era=${4:-"Full"}
outtext=${5:-"limit1D"}
outdir=${6:-"ntuples_v4/limits"}

## run macro
root -l -b -q runLimits1D.C\(\"${indir}\",\"${infilename}\",${doobserved},\"${era}\",\"${outtext}\"\)

## make outdirs readable
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy everything
for ext in "${exts[@]}"
do
    cp "${outtext}"*".${ext}" "${fulldir}"
done
cp "${outtext}.root" "${fulldir}"

## Final message
echo "Finished RunningLimits1D"
