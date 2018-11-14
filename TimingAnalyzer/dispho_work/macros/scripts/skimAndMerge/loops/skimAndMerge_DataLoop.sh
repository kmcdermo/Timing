#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1:-"data"}
dataset=${2:-"SinglePhoton"}

for subera in "B v1" "C v1" "D v1" "E v1" "F v1"
do echo ${subera} | while read -r era version
    do
	nohup ./scripts/skimAndMerge/skimAndMerge_DataEra.sh ${label} ${dataset} ${era} ${version} >& ${dataset}_${era}_${version}_Skim.log &
    done
done
