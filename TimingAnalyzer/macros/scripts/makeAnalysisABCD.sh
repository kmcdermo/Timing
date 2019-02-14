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
is_blind=${2:-"true"}
use_obs=${3:-"false"}
savemetadata=${4:-0}
docleanup=${5:-"true"}
x=${6-""}
y=${7-""}
# x1=${6-""}
# x2=${7-""}
# y1=${8-""}
# y2=${9-""}

## Scan config
ws_filename="ws_inputs.root"

## Limit config
inlimitdir="input"

######################
## Make Directories ##
######################

mkdir -p "${limitdir}/${inlimitdir}"

################################################
## Make 2D Input Plots from Significance Scan ##
################################################

./scripts/makePlotsFromSigScan.sh "${outdir}" "${is_blind}" "${ws_filename}" ${savemetadata} "${docleanup}" "${x}" "${y}"
#./scripts/makePlotsFromSigScan.sh "${outdir}" "${is_blind}" "${ws_filename}" ${savemetadata} "${docleanup}" "${x1}" "${x2}" "${y1}" "${y2}"

###############################
## Copy input into limit dir ##
###############################

cp "${ws_filename}" "${limitdir}/${inlimitdir}"

#########################
## Move Into Limit Dir ##
#########################

pushd "${limitdir}"

#########################
## Run Combine + Plots ##
#########################

./scripts/makeLimitsABCD.sh "${inlimitdir}" "${ws_filename}" "${use_obs}" "${outdir}" ${savemetadata} "${docleanup}"

#########################
## Snap Back When Done ##
#########################

popd

###################
## Final Message ##
###################

echo "Finished making limits in full script (ABCD)"
