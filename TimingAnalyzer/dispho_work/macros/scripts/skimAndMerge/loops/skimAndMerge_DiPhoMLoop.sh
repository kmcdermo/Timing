#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}
tune=${2}

for sample in M40_80 MGG-80toInf
do
    ./scripts/skimAndMerge/skimAndMerge_DiPhoMBin.sh ${label} ${tune} ${sample}
done
