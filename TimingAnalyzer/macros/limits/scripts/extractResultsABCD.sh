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
ws_outfile_base=${2:-"ws"}
use_obs=${3:-"false"}
outcombname=${4:-"AsymLim"}
outlimitdir=${5:-"output"}
outdir=${6:-"limits"}

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

cp "${inlimitdir}/${base_datacardABCD}"*".txt" "${combdir}"
cp "${ws_outfile_base}_L"*.root "${combdir}"
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
    echo "Doing lambda: ${lambda}"
    ./extractResultsABCDSub.sh "${lambda}" "${combine_extra}" "${logname}" &
done

wait

############
## rename ##
############

rename "higgsCombine" "${outcombname}" *.root
rename ".AsymptoticLimits.mH120" "" *.root

################
## Move back! ##
################

popd
eval `scram runtime -sh`

###########################################
## Ship things over to combine directory ##
###########################################

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"
cp "${combdir}/${logname}"*".${outTextExt}" "${fulldir}"
cp "${combdir}/${outcombname}"*.root "${fulldir}"

###################
## Final message ##
###################

echo "Finished ExtractingResultsABCD"
