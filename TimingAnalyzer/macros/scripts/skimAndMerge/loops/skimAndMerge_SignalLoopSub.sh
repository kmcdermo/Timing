#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1:-"gmsb"}
group=${2:-"GMSB"}
lambda=${3:-"100"}
tune=${4:-"TuneCP5_13TeV-pythia8"}

## GMSB Ctaus
for ctau in 0p001 0p1 10 200 400 600 800 1000 1200 10000
do
    if [[ "${ctau}" == "0p1" ]]
    then
	if [[ "${lambda}" == "500" ]] || [[ "${lambda}" == "600" ]]
	then
	    sctau="${ctau}"
	else
	    sctau="0_1";
	fi
    else
	sctau="${ctau}"
    fi
    
    gridpoint="L-${lambda}TeV_Ctau-${sctau}cm"
    nohup ./scripts/skimAndMerge/skimAndMerge_SignalGridPoint.sh ${label} ${group} ${gridpoint} ${tune} >& ${group}_${gridpoint}_Skim.log
done

