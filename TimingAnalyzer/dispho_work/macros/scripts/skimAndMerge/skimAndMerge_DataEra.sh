#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}
dataset=${2}
era=${3}
version=${4}

## global vars
database="Run2017${era}-17Nov2017-${version}"
text="${dataset}_2017${era}_${version}"

## directories needed
indir="${inbase}/analysis/${dataset}/${label}_${database}"
tmpdir="${tmpbase}/${dataset}/${era}/${version}"
outdir="${outbase}/Data/${dataset}/${era}/${version}"
	
## process subera
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir} 0

