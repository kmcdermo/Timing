#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
tmplimitdir=${1:-"tmp"}
inplotsfile=${2:-"met_vs_time_box_SR.root"}
doobs=${2:-0}
outdir=${3:-"madv2_v3/checks_v1"}
docleanup=${4:-"true"}

## Combine config
inlimitdir="input"
outcombname="AsymLim"
outlimitdir="output"

## Limit plot config
limit="limits2D"
outlimit1D="limit1D"
outlimit2D="limit2D"
outlimitplotdir="limits"

######################
## Make Directories ##
######################

mkdir -p "${inlimitdir}"
mkdir -p "${outlimitdir}"

##########################
## Make Datacards + WSs ##
##########################

echo "Making Datacards and WSs"
./scripts/makeDatacardsAndWSs.sh "${tmplimitdir}" "${inplotsfile}" "${inlimitdir}"

###########################################
## Extract Limits From WS : Run Combine! ##
###########################################

echo "Extracting Results ABCD"
./scripts/extractResultsABCD.sh "${inlimitdir}" "${outcombname}" "${outlimitdir}" 

#########################
## Make 1D Limit Plots ##
#########################

echo "Running limits 1D"
./scripts/runLimits1D.sh "${outlimitdir}" "${outcombname}" ${doobs} "${MainEra}" "${outlimit1D}" "${outdir}/${outlimitplotdir}"

#########################
## Make 2D Limit Plots ##
#########################

echo "Running limits 2D"
./scripts/runLimits2D.sh "${outlimitdir}" "${outcombname}" "${limitconfigdir}/${limit}.${inTextExt}" "${MainEra}" "${outlimit2D}" "${outdir}/${outlimitplotdir}"

###########################
## Clean Up If Requested ##
###########################

if [[ "${docleanup}" == "true" ]]; then
    echo "Cleaning up tmp dirs: ${tmplimitdir}, ${inlimitdir}, ${outlimitdir}"
    rm -r "${tmplimitdir}"
    rm -r "${inlimitdir}"
    rm -r "${outlimitdir}"
fi

###################
## Final Message ##
###################

echo "Finished making limit plots (ABCD)"
