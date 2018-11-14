#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1:-"bkgd"}
tune=${2:-"TuneCP5_13TeV-madgraphMLM-pythia8"}

for bin in 100To200 200To400 400To600 600To800 800To1200 1200To2500 2500ToInf
do
    nohup ./scripts/skimAndMerge/skimAndMerge_ZJetsHTBin.sh ${label} ${tune} ${bin} >& ZJets_HT_${bin}_Skim.log &
done
