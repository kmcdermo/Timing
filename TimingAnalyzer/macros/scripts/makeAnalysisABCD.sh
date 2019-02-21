#!/bin/bash

##################
## Source First ##
##################
source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
xbin=${1:-""}
xblind=${2:-""}
ybin=${3:-""}
yblind=${4:-""}
outdir=${5:-"madv2_v3/full_chain/results_ABCD"}
is_blind=${6:-"true"}
use_obs=${7:-"false"}
save_meta_data=${8:-0}
do_cleanup=${9:-"true"}

## Plot name
plotfiletext="met_vs_time"
plotfilename="${plotfiletext}.root"

################################################
## Make 2D Input Plots from Significance Scan ##
################################################

./scripts/makePlotsForABCD.sh "${xbin}" "${xblind}" "${ybin}" "${yblind}" "${plotfiletext}" "${outdir}" "${is_blind}" ${save_meta_data} "${do_cleanup}"

###############################
## Copy input into limit dir ##
###############################

cp "${plotfilename}" "${limitdir}"

#########################
## Move Into Limit Dir ##
#########################

pushd "${limitdir}"

#########################
## Run Combine + Plots ##
#########################

./scripts/makeLimitsABCD.sh "${plotfilename}" "${outdir}" "${is_blind}" "${use_obs}" ${save_meta_data} "${do_cleanup}"

#########################
## Snap Back When Done ##
#########################

popd

###################
## Final Message ##
###################

echo "Finished making limits in full script (ABCD)"
