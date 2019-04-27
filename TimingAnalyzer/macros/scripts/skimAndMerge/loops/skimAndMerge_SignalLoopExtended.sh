#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1:-"gmsb_ext"}
tune=${2:-"13TeV-pythia8"}

## inner loop
function SignalLoopExtendedSub ()
{
    local label=${1}
    local group=${2}
    local lambda=${3}
    local tune=${4}

    ## loop over ctaus, submit one at a time
    for ctau in 1 50 100
    do
	local gridpoint="L${lambda}TeV_Ctau${ctau}cm"
	
	echo "Skimming in parallel for gridpoint: ${gridpoint}" 
	nohup ./scripts/skimAndMerge/skimAndMerge_SignalGridPoint.sh ${label} ${group} ${gridpoint} ${tune} >& ${group}_${gridpoint}_Skim.log &
	wait # submit one signal point at a time before doing the next one
    done

    echo "Finished loop over ctaus!"
}
export -f SignalLoopExtendedSub

## GMSB Reduced Main Outer Loop
for lambda in 100 150 200 250 300 350 400
do
    echo "Skimming in parallel for lambda: ${lambda}"
    nohup bash -c "SignalLoopExtendedSub ${label} GMSB ${lambda} ${tune}" >& ${lambda}_Skims.log &
done
