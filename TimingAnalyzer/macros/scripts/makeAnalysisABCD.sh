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
use_obs=${7:-"true"}
use_systematics=${8:-"false"}
make_limit_plots=${9:-"false"}
save_meta_data=${10:-0}
do_cleanup=${11:-"true"}

## Derived Input
if [[ "${use_obs}" == "true" ]]
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
./scripts/makePlotsForABCD.sh "${xbin}" "${xblind}" "${ybin}" "${yblind}" "${outdir}" "${is_blind}" ${save_meta_data} "${do_cleanup}"

####################
## Make Datacards ##
####################

echo "Making Datacards ABCD"
./scripts/makeDatacardsABCD.sh "${outdir}" "${is_blind}" "${use_systematics}" "${do_cleanup}"

##################################################
## Extract Limits From Datacards : Run Combine! ##
##################################################

echo "Extracting Results ABCD"
./scripts/extractResultsABCD.sh "${outdir}" "${use_obs}" "${do_cleanup}"

######################
## Exclusion curves ##
######################

if [[ "${make_limit_plots}" == "true" ]]
then
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
fi    

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
