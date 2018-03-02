#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}
tune=${2}

for bin in 40To100 100To200 200To400 400To600 600ToInf
do
    ./scripts/skimAndMerge/skimAndMerge_GJetsHTBin.sh ${label} ${tune} ${bin}
done
