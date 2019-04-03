#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1:-"gmsb"}
tune=${2:-"TuneCP5_13TeV-pythia8"}

## GMSB Reduced
for lambda in 100 150 200 250 300 350 400
do
    nohup ./scripts/skimAndMerge/loops/skimAndMerge_SignalLoopSub.sh ${label} "GMSB" ${lambda} ${tune} &
done
