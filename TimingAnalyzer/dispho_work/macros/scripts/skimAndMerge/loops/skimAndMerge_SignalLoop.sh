#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}

for ctau in 0p1 10
do
    for lambda in 100 150 200 250 300 350 400
    do
	gridpoint="L${lambda}TeV_CTau${ctau}cm"
	./scripts/skimAndMerge/skimAndMerge_SignalGridPoint.sh ${label} "GMSB" ${gridpoint} "_step0"
    done
done

./scripts/skimAndMerge/skimAndMerge_SignalGridPoint.sh ${label} "GMSB" "L200TeV_CTau400cm" "_930"
