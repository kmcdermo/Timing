#!/bin/bash

## source first
source scripts/common_variables.sh

## config
nbinsX=${1:-2}
nbinsY=${2:-2}
outdir=${3:-"madv2_v3/checks_v13"}

## run macro
root -l -b -q runABCDGenerator.C\(${nbinsX},${nbinsY}\)

## make outdirs readable
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy configs
cp "${bininfo}" "${ratioinfo}" "${binratioinfo}" "${fulldir}"

## Final message
echo "Finished RunningLimits1D"
