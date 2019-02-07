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
ws_filename=${2:-"ws_final.root"}
use_obs=${3:-"false"}
outcombname=${4:-"AsymLim"}
outlimitdir=${5:-"output"}

## derived params : run expected limits?
if [[ "${use_obs}" == "true" ]]
then
    combine_extra=""
else
    combine_extra="--run=blind"
fi

###########################################
## Ship things over to combine directory ##
###########################################

cp "${inlimitdir}/${base_datacardABCD}"*".txt" "${combdir}"
cp "${inlimitdir}/${ws_filename}" "${combdir}"

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
    for ctau in 0p001 0p1 10 200 400 600 800 1000 1200 10000
    do
	sample="GMSB_L${lambda}_CTau${ctau}"
	echo "Working on ${sample}"

	combine -M AsymptoticLimits "${base_datacardABCD}_${sample}.${inTextExt}" "${combine_extra}" --name "${sample}"
    done
done

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

cp "${combdir}/${outcombname}"*.root "${outlimitdir}"

###################
## Final message ##
###################

echo "Finished ExtractingResultsABCD"
