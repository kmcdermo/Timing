#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}
tune=${2}

for sample in base ext
do
    ./scripts/skimAndMerge/skimAndMerge_DYLLHTBin.sh ${label} ${tune} ${sample}
done