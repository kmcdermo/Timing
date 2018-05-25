#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}
tune=${2}
sample=${3}

## global vars
mcbase="VGamma"
sample_label=${sample}
if [[ "${samle}" = *"ToLLGG"* ]] ; then
    sample_label="ZGGJets_ZToLL"
fi
text="${mcbase}_${label}"

## directories needed
indir="${inbase}/analysis/${sample}_${tune}/${label}_${sample}_${tune}"
tmpdir="${tmpbase}/${mcbase}/${sample_label}"
outdir="${outbase}/MC/${mcbase}/${sample_label}"

## process this bin
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir} 1
