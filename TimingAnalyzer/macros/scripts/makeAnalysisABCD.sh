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
outlimitdir=${5:-"output"}
outlimitplotdir=${6:-"limits"}
combinelogname=${7:-"combine"}
outcombname=${8:-"AsymLim"}
outdir=${9:-"madv2_v3/full_chain/results_ABCD"}

is_blind=${10:-"true"}
use_obs=${11:-"true"}
use_systematics=${12:-"false"}
save_meta_data=${13:-0}
do_cleanup=${14:-"true"}

## 2D Plot Name
plotfiletext="met_vs_time"
plotfilename="${plotfiletext}.root"

## ABCD Config
datacardname="datacardABCD"
incombdir="combine_input" # for www

## Combine config
inlimitdir="input"

## Limit plot config
limit="limits2D"
outlimit1D="limit1D"
outlimit2D="limit2D"

## Derived Input
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

################################################
## Make 2D Input Plots from Significance Scan ##
################################################

echo "Making Plots for ABCD"
./scripts/makePlotsForABCD.sh "${xbin}" "${xblind}" "${ybin}" "${yblind}" "${plotfiletext}" "${outdir}" "${is_blind}" ${save_meta_data} "${do_cleanup}"

####################
## Make Datacards ##
####################

echo "Making Datacards ABCD"
./scripts/makeDatacardsABCD.sh "${plotfilename}" "${datacardname}" "${outdir}/${incombdir}" "${inlimitdir}" "${is_blind}" "${use_systematics}" "${do_cleanup}"

##################################################
## Extract Limits From Datacards : Run Combine! ##
##################################################

echo "Extracting Results ABCD"
./scripts/extractResultsABCD.sh "${inlimitdir}" "${datacardname}" "${outdir}/${outlimitplotdir}" "${outcombname}" "${outlimitdir}" "${combinelogname}" "${use_obs}" "${do_cleanup}"

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

echo "Finished making limits in full script (ABCD)"
