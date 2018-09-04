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

## lauch plots without corrections
./scripts/makeTimePlots.sh "${outdirbase}/no_corrs" "false"

## extract corrections
./scripts/runTimeExtractor.sh

## lauch plots with corrections
./scripts/makeTimePlots.sh "${outdirbase}/with_corrs" "true"