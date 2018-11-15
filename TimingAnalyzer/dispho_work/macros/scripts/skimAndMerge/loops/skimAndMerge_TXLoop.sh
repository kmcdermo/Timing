#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1:-"bkgd"}

## sample pairs
TTJets="TTJets TuneCP5_13TeV-amcatnloFXFX-pythia8"
TGJets="TuneCP5_13TeV_amcatnlo_madspin_pythia8"
TTGJets="TuneCP5_13TeV-amcatnloFXFX-madspin-pythia8"
ttZJets="TuneCP5_13TeV_madgraphMLM_pythia8"
ttWJets="TuneCP5_13TeV_madgraphMLM_pythia8"
TGGJets="TuneCP5_PSweights_13TeV-MadGraph-madspin-pythia8"

## all samples together now
declare -a samples=(TTJets TGJets TTGJets ttZJets ttWJets TGGJets)

## loop time
for sample in "${samples[@]}"
do
    nohup ./scripts/skimAndMerge/skimAndMerge_TX.sh ${label} ${sample} ${!sample} >& TX_${sample}_Skim.log &
done