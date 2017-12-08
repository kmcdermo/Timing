#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}

for subera in "E v1" "F v1"
do echo ${subera} | while read -r era version
    do
	./scripts/skimAndMerge/skimAndMerge_DataEra.sh ${label} ${era} ${version}
    done
done
