#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}
tune=${2}

for bin in 100to200 200to400 400to600 600to800 800to1200 1200to2500 2500toInf
do
    ./scripts/skimAndMerge/skimAndMerge_DYLLHTBin.sh ${label} ${tune} ${bin}
done
