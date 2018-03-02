#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}
extra=${2:-""}

for sample in "GMSB L200TeV_CTau400cm"
do echo ${sample} | while read -r model gridpoint
    do
	./scripts/skimAndMerge/skimAndMerge_SignalGridPoint.sh ${label} ${model} ${gridpoint} ${extra}
    done
done
