#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}
dataset=${2}

for subera in "B v1" "B v2"
do echo ${subera} | while read -r era version
    do
	./scripts/skimAndMerge/skimAndMerge_DataEra.sh ${label} ${dataset} ${era} ${version}
    done
done