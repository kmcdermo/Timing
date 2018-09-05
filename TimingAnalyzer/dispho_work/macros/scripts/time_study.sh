#!/bin/bash

##################
## Source First ##
##################
source scripts/common_variables.sh

############
## Config ##
############

## command line inputs
outdirbase=${1:-"plots/ntuples_v4/checks_v4/era_plots"}

## make generic plots
./scripts/make1Dplots.sh "${outdirbase}/basic_plots" "standard_plots_zee" "false"

## lauch plots without corrections
./scripts/makeTimePlots.sh "${outdirbase}/no_corrs" "false"

## launch time adjuster
./scripts/runTimeAdjuster.sh

## lauch plots with corrections
./scripts/makeTimePlots.sh "${outdirbase}/with_corrs" "true"
