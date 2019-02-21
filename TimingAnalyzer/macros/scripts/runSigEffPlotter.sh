#!/bin/bash

## source first
source scripts/common_variables.sh

## config
infilename=${1:-"signal_skims.root"}
outtext=${2:-"signal_efficiency"}
outdir=${3:-"sigeffplots"}

## run macro
root -l -b -q runSigEffPlotter.C\(\"${infilename}\",\"${outtext}\"\)

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy everything
for canvscale in "${canvscales[@]}"
do
    for ext in "${exts[@]}"
    do
	cp "${outtext}_${canvscale}.${ext}" "${fulldir}"
    done
done
cp "${outtext}.root" "${fulldir}"

## Final message
echo "Finished ComputingSignalEfficiency"
