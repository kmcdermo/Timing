#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}

for bin in 100to200 200to300 300to500 500to700
do
    ./scripts/skimAndMerge/skimAndMerge_QCDHTBin.sh ${label} ${bin}
done
