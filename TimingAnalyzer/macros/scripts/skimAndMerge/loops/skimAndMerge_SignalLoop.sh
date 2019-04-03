#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1:-"gmsb"}
tune=${2:-"TuneCP5_13TeV-pythia8"}

## inner loop
function SignalLoopSub ()
{
    local label=${1}
    local group=${2}
    local lambda=${3}
    local tune=${4}

    ## loop over ctaus, submit one at a time
    for ctau in 0p001 0p1 10 200 400 600 800 1000 1200 10000
    do
	if [[ "${ctau}" == "0p1" ]]
	then
	    if [[ "${lambda}" == "500" ]] || [[ "${lambda}" == "600" ]]
	    then
		local sctau="${ctau}"
	    else
		local sctau="0_1";
	    fi
	else
	    local sctau="${ctau}"
	fi
	local gridpoint="L-${lambda}TeV_Ctau-${sctau}cm"
	
	echo "Skimming in parallel for gridpoint: ${gridpoint}" 
	nohup ./scripts/skimAndMerge/skimAndMerge_SignalGridPoint.sh ${label} ${group} ${gridpoint} ${tune} >& ${group}_${gridpoint}_Skim.log &
	wait # submit one signal point at a time before doing the next one
    done
}
export -f SignalLoopSub

## GMSB Reduced Main Outer Loop
for lambda in 100 150 200 250 300 350 400
do
    echo "Skimming in parallel for lambda: ${lambda}"
    nohup bash -c "SignalLoopSub ${label} GMSB ${lambda} ${tune}" >& ${lambda}_Skims.log &
done
