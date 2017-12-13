#!/bin/bash

source scripts/common_variables.sh

## input
label=${1}

## global vars
mcbase="DYJetsToLL"
bin="M-50"
text="${mcbase}_${bin}"

## directories needed
indir="${eosbase}/${mcbase}${bin}_${bkgdtune}/${label}_${mcbase}${bin}_${bkgdtune}"
tmpdir="${tmpbase}/${mcbase}/${bin}"
outdir="${outbase}/MC/${mcbase}/${bin}"

## process this bin
./scripts/skimAndMerge/processSkimAndMerge.sh ${text} ${indir} ${tmpdir} ${outdir}
