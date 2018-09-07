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
inwsfile=${2:-"ws_final.root"}
doobs=${3:-0}
outdir=${4:-"ntuples_v4/full_chain"}
docleanup=${5:-"true"}

## Combine config
outcombname="AsymLim"
outlimitdir="output"

## Limit plot config
limit="limits2D"
outlimit1D="limit1D"
outlimit2D="limit2D"
outlimitplotdir="limits"

###############################################
## Extract Limits From Fitter : Run Combine! ##
###############################################

./scripts/extractResults.sh "${inlimitdir}" "${inwsfile}" "${outcombname}" "${outlimitdir}" 

#########################
## Make 1D Limit Plots ##
#########################

./scripts/runLimits1D.sh "${outlimitdir}" "${outcombname}" ${doobs} "${MainEra}" "${outlimit1D}" "${outdir}/${outlimitplotdir}"

#########################
## Make 2D Limit Plots ##
#########################

./scripts/runLimits2D.sh "${outlimitdir}" "${outcombname}" "${limitconfigdir}/${limit}.${inTextExt}" "${MainEra}" "${outlimit2D}" "${outdir}/${outlimitplotdir}"

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

echo "Finished making limit plots"
