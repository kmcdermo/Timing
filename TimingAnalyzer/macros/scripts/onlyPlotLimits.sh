#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
outdir=${1:-"madv2_v3/full_chain/results_ABCD"}

use_obs=${2:-"false"}
save_meta_data=${3:-0}
do_cleanup=${4:-"false"}

## derived input
if [[ "${use_obs}" == "true" ]]
then
    doobs=1
else
    doobs=0
fi

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

###################
## Final Message ##
###################

echo "Finished only making limit plots (ABCD)"
