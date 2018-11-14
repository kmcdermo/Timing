#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1:-"bkgd"}

## sample pairs
WW="TuneCP5_13TeV-pythia8"
WZ="TuneCP5_13TeV-pythia8"
WGGJets="TuneCP5_13TeV_madgraphMLM_pythia8"
WWG="TuneCP5_13TeV-amcatnlo-pythia8"
WWW="4F_TuneCP5_13TeV-amcatnlo-pythia8"
WWZ="4F_TuneCP5_13TeV-amcatnlo-pythia8"
WZZ="TuneCP5_13TeV-amcatnlo-pythia8"
WZG="TuneCP5_13TeV-amcatnlo-pythia8"

## all samples together now
declare -a samples=(WW WZ WGGJets WWG WWW WWZ WZZ WZG)

## loop time
for sample in "${sample[@]}"
do
    nohup ./scripts/skimAndMerge/skimAndMerge_WX.sh ${label} ${sample} ${!sample} >& WX_${sample}_Skim.log &
done