#!/bin/bash

source scripts/skimAndMerge/common_variables.sh

## input
label=${1}
tune=${2}
sample=${3}
skimtype=${4:-"Standard"}

## other input vars
usePUWeights=1

## global vars
mcbase="DYJetsToLL"
extra="M-50"
mcfull="${mcbase}_${extra}"
text="${mcfull}_${sample}"

## directories needed
indir="${inbase}/analysis/${mcfull}_${tune}/${label}_${mcfull}_${tune}"
if [ "${sample}" == "ext" ] ; then
    indir+="_"${sample}
fi
tmpdir="${tmpbase}/${mcbase}/${sample}"
outdir="${outbase}/MC/${mcbase}/${sample}"

## process this bin
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir} ${usePUWeights} ${skimtype}
