#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}
tune=${2}

for sample in base ext
do
    ./scripts/skimAndMerge/skimAndMerge_DYLLInclusiveSample.sh ${label} ${tune} ${sample}
done
