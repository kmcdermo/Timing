#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"plots/ntuples_v4/checks_v3/full_chain"}
docleanup=${2:-"true"}

## make signal efficiencies
echo "Making Signal Efficiencies"
./scripts/makeSignalEffs.sh "${outdir}/sig_effs"

## make Data/MC plots (no weights yet)
echo "Making 1D Data/MC plots with no weights"
./scripts/make1Dplots.sh "${outdir}/data_over_mc" "${reducedplotlist}" "false"

## make weights and related plots
echo "Making variable event weights and plots"
./scripts/makeWgtsAndPlots.sh "${outdir}/varwgts" "${docleanup}"

## make SR plots
echo "Making signal region plots"
./scripts/makePlotsForSR.sh "${outdir}/srplots" "${reducedplotlist}" "${docleanup}"

## make limit plots
echo "Making limit plots"
./scripts/makeAnalysis.sh "${outdir}/results" "${docleanup}"

## final make readable
echo "Final prep outdir"
PrepOutDir "${topdir}/${disphodir}/${outdir}"

## all done
echo "Finished full chain of analysis"
