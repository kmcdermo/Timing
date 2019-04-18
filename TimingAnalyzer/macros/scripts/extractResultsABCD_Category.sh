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

## tmp log base
tmp_log_base="tmp_lambda"

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

cp -r "${inlimitdir}" "${combdir}"

#####################
## Now work there! ##
#####################

pushd "${combdir}"
eval `scram runtime -sh`

###########################
## Inner loop over ctaus ##
###########################

function extractResultsSub ()
{
    ## i/o params
    local lambda=${1:-"100"}

    ## perfom limits with combine!
    for ctau in 10 200 400 600 800 1000 1200 10000
    do
	local sample="GMSB_L${lambda}_CTau${ctau}"
	echo "Running combine for: ${sample}"
	
	local log="${combinelogname}_${sample}.${outTextExt}"
	
	## first, combine cards
	combineCards.py "${inlimitdir}/${ex1pho}/${datacardname}_${sample}.${inTextExt}" "${inlimitdir}/${in2pho}/${datacardname}_${sample}.${inTextExt}" > "${datacardname}_${sample}.${inTextExt}" 

	## then, run combine over combined card
	combine -M AsymptoticLimits "${datacardname}_${sample}.${inTextExt}" ${combine_extra} --name "${sample}" >& "${log}"
    done
}
export -f extractResultsSub

#################################################
## Loop over signals and run asymptotic limits ##
#################################################

for lambda in 100 150 200 250 300 350 400
do
    echo "Extracting results for lambda: ${lambda}"
    extractResultsSub "${lambda}" &
done

## wait for all combine jobs to finish before moving onto the next steps
wait

###########################
## Rename Combine Output ##
###########################

rename "higgsCombine" "${outcombname}" *".root"
rename ".AsymptoticLimits.mH120" "" *".root"

################
## Move back! ##
################

popd
eval `scram runtime -sh`

###########################################
## Ship things over to combine directory ##
###########################################

cp "${combdir}/${outcombname}"*".root" "${outlimitdir}"

################################
## Move combine output to web ##
################################

fullinputdir="${topdir}/${disphodir}/${outdir}/${incombdir}"
PrepOutDir "${fullinputdir}"
cp "${combdir}/${datacardname}"*".${inTextExt}" "${fullinputdir}"

fulllimitdir="${topdir}/${disphodir}/${outdir}/${outlimitplotdir}"
PrepOutDir "${fulllimitdir}"
cp "${combdir}/${combinelogname}"*".${outTextExt}" "${fulllimitdir}"
cp "${combdir}/${outcombname}"*".root" "${fulllimitdir}"

###########################
## Clean up if requested ##
###########################

if [[ "${do_cleanup}" == "true" ]]
then
    echo "Cleaning up combine dir"
    rm "${combdir}/${datacardname}"*".${inTextExt}"
    rm "${combdir}/${combinelogname}"*".${outTextExt}"
    rm "${combdir}/${outcombname}"*".root"
fi

###################
## Final message ##
###################

echo "Finished ExtractingResultsABCD"
