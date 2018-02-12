#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}
tune=${2}
sample=${3}

## global vars
mcbase="DYJetsToLL_M-50"
text="${mcbase}_${sample}"

## directories needed
indir="${inbase}/${mcbase}_${tune}/${label}_${mcbase}_${tune}"
if [ "${sample}" == "ext" ] ; then
    indir+="_"${sample}
fi
tmpdir="${tmpbase}/${mcbase}/${sample}"
outdir="${outbase}/MC/${mcbase}/${sample}"

## process this bin
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir}
