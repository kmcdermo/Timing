#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"madv2_v3/checks_v1/full_chain"}
docleanup=${2:-"true"}

## make signal efficiencies
echo "Making Signal Efficiencies"
./scripts/makeSignalEffs.sh "${outdir}/sig_effs"

## make Data/MC plots (no weights yet)
echo "Making 1D Data/MC plots with no weights"
./scripts/make1Dplots.sh "${outdir}/data_over_mc" "${reducedplotlist}" "false"

## make limit plots
echo "Making limit plots"
./scripts/makeAnalysisABCD.sh "${outdir}/results" "${docleanup}"

## final prep dir
echo "Final prep outdir"
PrepOutDir "${topdir}/${disphodir}/${outdir}"

## all done
echo "Finished full chain of analysis (ABCD)"
