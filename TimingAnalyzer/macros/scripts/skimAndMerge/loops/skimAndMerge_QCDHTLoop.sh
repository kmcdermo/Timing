#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1:-"bkgd"}
tune=${2:-"TuneCP5_13TeV-madgraph-pythia8"}

for bin in 100to200 200to300 300to500 500to700 700to1000 1000to1500 1500to2000 2000toInf
do
    nohup ./scripts/skimAndMerge/skimAndMerge_QCDHTBin.sh ${label} ${tune} ${bin} >& QCD_HT_${bin}_Skim.log &
done
