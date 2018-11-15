#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1:-"bkgd"}
tune=${2:-"Sherpa"}

for sample in M40_80 MGG-80toInf
do
    nohup ./scripts/skimAndMerge/skimAndMerge_DiPhoMBin.sh ${label} ${tune} ${sample} >& DiPho_${sample}_Skim.log &
done
