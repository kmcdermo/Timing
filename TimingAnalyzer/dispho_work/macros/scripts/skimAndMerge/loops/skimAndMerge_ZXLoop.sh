#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1:-"bkgd"}

## sample pairs
ZZ="TuneCP5_13TeV-pythia8"
ZGGJets="ZToHadOrNu_5f_LO_madgraph_pythia8"
ZZZ="TuneCP5_13TeV-amcatnlo-pythia8"

## all samples together now
declare -a samples=(ZZ ZGGJets ZZZ)

## loop time
for sample in "${sample[@]}"
do
    nohup ./scripts/skimAndMerge/skimAndMerge_ZX.sh ${label} ${sample} ${!sample} >& ZX_${sample}_Skim.log &
done