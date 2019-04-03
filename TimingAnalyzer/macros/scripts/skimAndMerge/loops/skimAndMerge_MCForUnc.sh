#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1:-"gmsb"}
tune=${2:-"TuneCP5_13TeV-pythia8"}

## GMSB Reduced
for lambda in 100 200 300
do
    for ctau in 10 200 1000
    do
	gridpoint="L-${lambda}TeV_Ctau-${ctau}cm"
	./scripts/skimAndMerge/skimAndMerge_SignalGridPoint.sh ${gmsblabel} "GMSB" ${gridpoint} ${gmsbtune}
    done
done
