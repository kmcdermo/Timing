#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
inlimitdir=${1:-"input"}
ws_filename=${2:-"ws_inputs.root"}
use_obs=${3:-"false"}
outdir=${4:-"madv2_v3/full_chain/results_ABCD"}
savemetadata=${5:-0}
docleanup=${6:-"true"}

## Card config
outcarddir="combine_input"

## Combine config
ws_outfile_base="ws"
outcombname="AsymLim"
outlimitdir="output"

## Limit plot config
limit="limits2D"
outlimit1D="limit1D"
outlimit2D="limit2D"
outlimitplotdir="limits"

## derived input
if [[ "use_obs" == "true" ]]
then
    doobs=1
else
    doobs=0
fi

######################
## Make Directories ##
######################

mkdir -p "${outlimitdir}"

##########################
## Make Datacards + WSs ##
##########################

echo "Making Datacards and WSs"
./scripts/makeDatacardsAndWSs.sh "${inlimitdir}" "${ws_filename}" "${outdir}/${outcarddir}" "${docleanup}" "${ws_outfile_base}"

###########################################
## Extract Limits From WS : Run Combine! ##
###########################################

echo "Extracting Results ABCD"
./scripts/extractResultsABCD.sh "${inlimitdir}" "${ws_outfile_base}" "${use_obs}" "${outcombname}" "${outlimitdir}" "${outdir}/${outlimitplotdir}"

###########################
## Clean Up If Requested ##
###########################

if [[ "${docleanup}" == "true" ]]; then
    echo "Cleaning up tmp dirs: ${inlimitdir}, ${outlimitdir}"
    rm -r "${inlimitdir}"
    rm -r "${outlimitdir}"
fi

###################
## Final Message ##
###################

echo "Finished making limit plots (ABCD)"
