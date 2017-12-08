#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}

for bin in 40To100 100To200 200To400
do
    ./scripts/skimAndMerge/skimAndMerge_GJetsHTBin.sh ${label} ${bin}
done
