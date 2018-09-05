#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}
dataset=${2}
skimtype=${3:-"Standard"}

for subera in "B v1" "C v1" "D v1" "E v1" "F v1"
do echo ${subera} | while read -r era version
    do
	nohup ./scripts/skimAndMerge/skimAndMerge_DataEra.sh ${label} ${dataset} ${era} ${version} ${skimtype} >& ${Dataset}_${era}_${version}_Skim.log &
    done
done
