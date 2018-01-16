#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}
dataset=${2}

for subera in "C v3" "D v1"
do echo ${subera} | while read -r era version
    do
	./scripts/skimAndMerge/skimAndMerge_DataEra.sh ${label} ${dataset} ${era} ${version}
    done
done