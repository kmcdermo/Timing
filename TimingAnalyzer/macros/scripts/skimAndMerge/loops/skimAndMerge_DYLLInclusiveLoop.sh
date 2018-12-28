#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1:-"bkgd"}
tune=${2:-"TuneCP5_13TeV-madgraphMLM-pythia8"}

for sample in base #ext
do
    nohup ./scripts/skimAndMerge/skimAndMerge_DYLLInclusiveSample.sh ${label} ${tune} ${sample} >& DYLL_${sample}_Skim.log &
done
