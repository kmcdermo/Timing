#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"plots/ntuples_v4/checks_v3/full_chain"}
docleanup=${2:-"true"}

## make Data/MC plots (no weights yet)
./scripts/make1Dplots.sh "${outdir}/data_over_mc"

## make weights and related plots
./scripts/makeWgtsAndPlots.sh "${outdir}/varwgts" ${docleanup}

## make SR plots
./scripts/makePlotsForSR.sh "${outdir}/srplots" ${docleanup}

## make limit plots
./scripts/makeAnalysis.sh "${outdir}/results" ${docleanup}

## final make readable
PrepOutDir "${outdir}"