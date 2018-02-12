#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}
tune=${2}
sample=${3}

## global vars
mcbase="DYJetsToLL"
extra="M-50"
mcfull="${mcbase}_${extra}"
text="${mcfull}_${sample}"

## directories needed
indir="${inbase}/${mcfull}_${tune}/${label}_${mcfull}_${tune}"
if [ "${sample}" == "ext" ] ; then
    indir+="_"${sample}
fi
tmpdir="${tmpbase}/${mcbase}/${sample}"
outdir="${outbase}/MC/${mcbase}/${sample}"

## process this bin
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir}
