#!/bin/bash

#######################
## Source This First ##
#######################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## i/o params
inlimitdir=${1:-"input"}
datacardname=${2:-"datacardABCD"}
outdir=${3:-"limits"}
outcombname=${4:-"AsymLim"}
outlimitdir=${5:-"output"}
combinelogname=${6:-"combine"}
use_obs=${7:-"false"}
do_cleanup=${8:-"true"}

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

for lambda in 150 200 250 300 350 400
do
    echo "Extracting results for lambda: ${lambda}"
    ./extractResultsABCDSub.sh "${lambda}" "${datacardname}" "${combinelogname}" "${use_obs}" &
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
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"
cp "${combdir}/${combinelogname}"*".${outTextExt}" "${fulldir}"
cp "${combdir}/${outcombname}"*".root" "${fulldir}"

###################
## Final message ##
###################

echo "Finished ExtractingResultsABCD"
