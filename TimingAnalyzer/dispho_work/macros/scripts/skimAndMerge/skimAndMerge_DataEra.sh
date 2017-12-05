#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}
era=${2}
version=${3}

## global vars
dataset="SinglePhoton"
database="Run2017${era}-PromptReco-${version}"
text="${dataset}_2017${era}_${version}"

## directories needed
indir="${eosbase}/${dataset}/${label}_${database}"
tmpdir="${tmpbase}/${dataset}/${era}/${version}"
outdir="${outbase}/Data/${dataset}/${era}/{version}"
	
## process subera
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir}
