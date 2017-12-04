#!/bin/bash

## input
label=${1}

for subera in "B v1" "B v2" "C v1" "C v2" "C v3" "D v1" "E v1" "F v1"
do echo ${subera} | while read -r era version
    do
	./mergeEOS_DataEra.sh ${label} ${era} ${version}
    done
done
