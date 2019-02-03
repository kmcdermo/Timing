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

## make signal efficiencies
echo "Making Signal Efficiencies"
./scripts/makeSignalEffs.sh "${outdir}/sig_effs"

## make Data/MC plots (no weights yet)
echo "Making 1D Data/MC plots with no weights"
./scripts/make1Dplots.sh "${outdir}/data_over_mc" "${reducedplotlist}" "false" ${savemetadata}

## make weights and related plots
echo "Making variable event weights and plots"
./scripts/makeWgtsAndPlots.sh "${outdir}/varwgts" ${savemetadata} "${docleanup}"

## make SR plots
echo "Making signal region plots"
./scripts/makePlotsForSR.sh "${outdir}/srplots" "${reducedplotlist}" ${savemetadata} "${docleanup}"

## make limit plots
echo "Making limit plots"
./scripts/makeAnalysis.sh "${outdir}/results" ${savemetadata} "${docleanup}"

## final prep dir
echo "Final prep outdir"
PrepOutDir "${topdir}/${disphodir}/${outdir}"

## all done
echo "Finished full chain of analysis"
