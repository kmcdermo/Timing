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
wsfilename=${2:-"ws_final.root"}
datacardname=${3:-"datacardABCD"}
outdir=${4:-"limits"}
outcombname=${5:-"AsymLim"}
outlimitdir=${6:-"output"}
use_obs=${7:-"false"}
do_cleanup=${8:-"true"}

## other global config
logname="combine"

## derived params : run expected limits?
if [[ "${use_obs}" == "true" ]]
then
    combine_extra=""
else
    combine_extra="--run=expected"
fi

###########################################
## Ship things over to combine directory ##
###########################################

cp "${inlimitdir}/${datacardname}"*".${inTextExt}" "${combdir}"
cp "${inlimitdir}/${wsfilename}" "${combdir}"
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

for lambda in 100 150 200 250 300 350 400 500 600
do
    echo "Extracting results for lambda: ${lambda}"
    ./extractResultsABCDSub.sh "${lambda}" "${datacardname}" "${combine_extra}" "${logname}" &
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
cp "${combdir}/${logname}"*".${outTextExt}" "${fulldir}"
cp "${combdir}/${outcombname}"*".root" "${fulldir}"

###################
## Final message ##
###################

echo "Finished ExtractingResultsABCD"
