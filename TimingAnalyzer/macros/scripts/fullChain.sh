#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"ntuples_v4/checks_v3/full_chain"}
savemetadata=${2:-0}
docleanup=${3:-"true"}

## for safety
echo "Compiling ahead of time"
./scripts/compile.sh

## make limit plots
echo "Making limit plots"
./scripts/makeAnalysis.sh "${outdir}/results" ${savemetadata} "${docleanup}"

## final prep dir
echo "Final prep outdir"
PrepOutDir "${topdir}/${disphodir}/${outdir}"

## all done
echo "Finished full chain of analysis"
