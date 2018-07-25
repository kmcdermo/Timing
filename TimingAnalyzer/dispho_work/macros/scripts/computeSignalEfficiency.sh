#!/bin/bash

## source first
source scripts/common_variables.sh

## config
infilename=${1:-"signal_skims.root"}
outtext=${2:-"signal_efficiency"}
dir=${3:-"plots"}

## run macro
root -l -b -q computeSignalEfficiency.C\(\"${infilename}\",\"${outtext}\"\)

## make out dirs
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir "${fulldir}"

## copy everything
for ext in "${exts[@]}"
do
    cp ${outtext}.${ext} ${fulldir}
done

## Final message
echo "Finished ComputingSignalEfficiency"
