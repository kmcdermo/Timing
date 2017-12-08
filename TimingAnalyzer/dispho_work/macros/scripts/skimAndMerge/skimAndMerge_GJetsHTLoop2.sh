#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}

for bin in 400To600 600ToInf
do
    ./scripts/skimAndMerge/skimAndMerge_GJetsHTBin.sh ${label} ${bin}
done
