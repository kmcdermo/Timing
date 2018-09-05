#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}
tune=${2}
skimtype=${3:-"Standard"}

for bin in 40To100 100To200 200To400 400To600 600ToInf
do
    nohup ./scripts/skimAndMerge/skimAndMerge_GJetsHTBin.sh ${label} ${tune} ${bin} ${skimtype} >& GJets_HT_${bin}_Skim.log &
done
