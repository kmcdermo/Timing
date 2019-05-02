#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
outdir=${1:-"madv2_v4/categories/v1p1/full_chain"}

use_obs=${2:-"true"}
use_systematics=${3:-"false"}
save_meta_data=${4:-0}
do_cleanup=${5:-"true"}

## Derived Input
if [[ "${use_obs}" == "true" ]]
then
    doobs=1
else
    doobs=0
fi

## Derived Config
fulldir="${topdir}/${disphodir}/${outdir}"

######################
## Make Directories ##
######################

mkdir -p "${inlimitdir}"
mkdir -p "${outlimitdir}"

##################################################
## Extract Limits From Datacards : Run Combine! ##
##################################################

echo "Collecting datacards"
./scripts/collectDatacards.sh "${outdir}"

##################################################
## Extract Limits From Datacards : Run Combine! ##
##################################################

echo "Extracting Results ABCD"
./scripts/extractResultsABCD_Category.sh "${outdir}/${categorydir}" "${use_obs}" "${do_cleanup}"

#########################
## Make 1D Limit Plots ##
#########################

echo "Running limits 1D"
./scripts/runLimits1D.sh "${outlimitdir}" "${outcombname}" ${doobs} "${MainEra}" "${outlimit1D}" "${outdir}/${categorydir}/${outlimitplotdir}"

#########################
## Make 2D Limit Plots ##
#########################

echo "Running limits 2D"
./scripts/runLimits2D.sh "${outlimitdir}" "${outcombname}" ${doobs} "${limitconfigdir}/${limit}.${inTextExt}" "${MainEra}" ${save_meta_data} "${outlimit2D}" "${outdir}/${categorydir}/${outlimitplotdir}"

###########################
## Clean Up If Requested ##
###########################

if [[ "${do_cleanup}" == "true" ]]
then
    echo "Cleaning up tmp dirs: ${inlimitdir}, ${outlimitdir}"
    rm -r "${inlimitdir}"
    rm -r "${outlimitdir}"

    echo "Final cleanup"
    ./scripts/cleanup.sh
fi

####################
## Final Prep Dir ##
####################

echo "Final prep outdir"
PrepOutDir "${fulldir}/${categorydir}"

###################
## Final Message ##
###################

echo "Finished making limits from categories (ABCD)"
