#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"madv2_v3/full_chain"}
is_blind=${2:-"true"}
use_obs=${3:-"false"}
docleanup=${4:-"true"}

## for safety
echo "Compiling ahead of time"
./scripts/compile.sh

## make signal efficiencies
echo "Making Signal Efficiencies"
./scripts/makeSignalEffs.sh "${outdir}/sig_effs"

## make Data/MC plots (no weights yet)
echo "Making 1D Data/MC plots with no weights"
./scripts/make1Dplots.sh "${outdir}/data_over_mc" "${reducedplotlist}" "false"

## make analysis plots + limits
echo "Making analysis plots + limits"
./scripts/makeAnalysisABCD.sh "${outdir}/ABCD" "${is_blind}" "${use_obs}" "${docleanup}"

## final prep dir
echo "Final prep outdir"
PrepOutDir "${topdir}/${disphodir}/${outdir}"

## all done
echo "Finished full chain of analysis (ABCD)"
