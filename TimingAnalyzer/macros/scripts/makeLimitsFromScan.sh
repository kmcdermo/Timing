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
do_cleanup=${4:-"true"}

################################
## Make Directories and Files ##
################################

mkdir -p "${outlimitdir}"
> "${scan_log}"

################################################
## Perform Scan For Best Expected R At Median ##
################################################

echo "Performing scan for best expected limit on r at median"
./scripts/scanLimits.sh "${x_log}" "${y_log}" "${scan_log}" "${outlimitplotdir}" "${combinelogname}" "${outdir}"

#############################################
## Copy Best Limit File for Each Gridpoint ##
#############################################

echo "Copying Best Limit File for Each Gridpoint"
./scripts/readScannedLimits.sh "${scan_log}" "${outlimitdir}" "${outlimitplotdir}" "${outcombname}"

######################
## Make Final Plots ##
######################

echo "Produce 1D and 2D Limits from Scan"
./scripts/onlyPlotLimits.sh "${outdir}" "${use_obs}" ${save_meta_data} "${do_cleanup}"

###########################
## Clean Up If Requested ##
###########################

if [[ "${do_cleanup}" == "true" ]]
then
    echo "Cleaning up tmp dirs and log files"
    rm -r "${outlimitdir}"
    rm "${scan_log}"
fi

###################
## Final Message ##
###################

echo "Finished Making Limit Plots From Scan"
