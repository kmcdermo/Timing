#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}
skimtype=${2:-"Standard"}

for ctau in 0p1 10 600 1200
do
    for lambda in 100 150 200 250 300 350 400
    do
	gridpoint="L${lambda}TeV_CTau${ctau}cm"
	nohup ./scripts/skimAndMerge/skimAndMerge_SignalGridPoint.sh ${label} "GMSB" ${gridpoint} ${skimtype} "_step0" >& GMSB_${gridpoint}_"step0"_Skim.log &
    done
done

nohup ./scripts/skimAndMerge/skimAndMerge_SignalGridPoint.sh ${label} "GMSB" "L200TeV_CTau400cm" ${skimtype} "_930" >& GMSB_"L200TeV_CTau400cm"_"930"_Skim.log &
