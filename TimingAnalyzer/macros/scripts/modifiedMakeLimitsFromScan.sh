#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
scan_log=${1:-"abcd_categories.log"}
outdir=${2:-"madv2_v3/full_chain/results_ABCD"}

use_obs=${3:-"false"}
save_meta_data=${4:-0}
do_cleanup=${5:-"true"}

################################
## Make Directories and Files ##
################################

mkdir -p "${outlimitdir}"

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
