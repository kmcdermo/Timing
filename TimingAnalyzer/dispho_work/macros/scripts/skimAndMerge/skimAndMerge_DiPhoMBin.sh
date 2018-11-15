#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}
tune=${2}
bin=${3}

## other input vars
usePUWeights=1

## global vars
mcbase="DiPhotonJetsBox"
binlabel=${bin}
if [[ "${bin}" = *"MGG-"* ]] ; then
    binlabel="M80_Inf"
    tune="_13TeV-"${tune}
else 
    tune="-"${tune}
fi
text="${mcbase}_${binlabel}"

## directories needed
indir="${inbase}/${mcbase}_${bin}${tune}/${label}_${mcbase}_${bin}${tune}"
tmpdir="${tmpbase}/${mcbase}/${binlabel}"
outdir="${outbase}/MC/${mcbase}/${binlabel}"

## process this bin
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir} ${usePUWeights}
