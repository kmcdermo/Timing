#!/bin/bash

#######################
## Source This First ##
#######################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
outdir=${1:-"madv2_v3/checks_v26"}

use_obs=${2:-"false"}
do_cleanup=${3:-"true"}

###########################################
## Ship things over to combine directory ##
###########################################

cp "${inlimitdir}/${datacardname}"*".${inTextExt}" "${combdir}"
cp "scripts/extractResultsABCDSub.sh" "${combdir}"
cp "scripts/common_variables.sh" "${combdir}"

#####################
## Now work there! ##
#####################

pushd "${combdir}"
eval `scram runtime -sh`

#################################################
## Loop over signals and run asymptotic limits ##
#################################################

for lambda in 100 150 200 250 300 350 400
do
    echo "Extracting results for lambda: ${lambda}"
    ./extractResultsABCDSub.sh "${lambda}" "${use_obs}" &
done

wait

###########################
## Rename Combine Output ##
###########################

rename "higgsCombine" "${outcombname}" *".root"
rename ".AsymptoticLimits.mH120" "" *".root"

###########################
## Clean Up If Requested ##
###########################

if [[ "${do_cleanup}" == "true" ]]
then
    echo "Cleaning up copied scripts"
    rm "extractResultsABCDSub.sh"
    rm "common_variables.sh"
fi

################
## Move back! ##
################

popd
eval `scram runtime -sh`

###########################################
## Ship things over to combine directory ##
###########################################

cp "${combdir}/${outcombname}"*".root" "${outlimitdir}"

##########################
## Store logs in outdir ##
##########################

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}/${outlimitplotdir}"
PrepOutDir "${fulldir}"
cp "${combdir}/${combinelogname}"*".${outTextExt}" "${fulldir}"
cp "${combdir}/${outcombname}"*".root" "${fulldir}"

###################
## Final message ##
###################

echo "Finished ExtractingResultsABCD"
