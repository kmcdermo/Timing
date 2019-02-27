#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
plotfilename=${1:-"met_vs_time.root"}
outdir=${2:-"madv2_v3/full_chain/results_ABCD"}
is_blind=${3:-"false"}
use_obs=${4:-"false"}
save_meta_data=${5:-0}
do_cleanup=${6:-"true"}

## ABCD Config
datacardname="datacardABCD"
incombdir="combine_input" # for www

## Combine config
inlimitdir="input"
outcombname="AsymLim"
outlimitdir="output"

## Limit plot config
limit="limits2D"
outlimit1D="limit1D"
outlimit2D="limit2D"
outlimitplotdir="limits" # for www

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

mkdir -p "${inlimitdir}"
mkdir -p "${outlimitdir}"

##########################
## Make WSs + Datacards ##
##########################

echo "Making Datacards"
./scripts/makeDatacardsABCD.sh "${plotfilename}" "${datacardname}" "${outdir}/${incombdir}" "${inlimitdir}" "${is_blind}" "${do_cleanup}"

###########################################
## Extract Limits From WS : Run Combine! ##
###########################################

echo "Extracting Results ABCD"
./scripts/extractResultsABCD.sh "${inlimitdir}" "${datacardname}" "${outdir}/${outlimitplotdir}" "${outcombname}" "${outlimitdir}" "${use_obs}" "${do_cleanup}"

#########################
## Make 1D Limit Plots ##
#########################

echo "Running limits 1D"
./scripts/runLimits1D.sh "${outlimitdir}" "${outcombname}" ${doobs} "${MainEra}" "${outlimit1D}" "${outdir}/${outlimitplotdir}"

#########################
## Make 2D Limit Plots ##
#########################

echo "Running limits 2D"
./scripts/runLimits2D.sh "${outlimitdir}" "${outcombname}" ${doobs} "${limitconfigdir}/${limit}.${inTextExt}" "${MainEra}" ${save_meta_data} "${outlimit2D}" "${outdir}/${outlimitplotdir}"

###########################
## Clean Up If Requested ##
###########################

if [[ "${do_cleanup}" == "true" ]]
then
    echo "Cleaning up tmp dirs: ${inlimitdir}, ${outlimitdir}"
    rm -r "${inlimitdir}"
    rm -r "${outlimitdir}"
fi

###################
## Final Message ##
###################

echo "Finished making limit plots (ABCD)"
