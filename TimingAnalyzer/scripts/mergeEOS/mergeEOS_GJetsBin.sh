#!/bin/bash

## input
label=${1}
bin=${2}

## global vars
base="2017/MC/gjets_HT"
tune="TuneCUETP8M1_13TeV-madgraphMLM-pythia8"
eosbase="/store/user/kmcdermo"
rootbase="root://eoscms"

## filenames
infiles="dispho_*.root"
outfile="tree.root"

## directories needed
indir="${eosbase}/GJets_HT-${bin}_${tune}/${label}_GJets_HT-${bin}_${tune}"
tmpdir="/tmp/kmcdermo/${base}"
bindir="HT_${bin}"
outdir="${eosbase}/nTuples/${base}/${bindir}"

## make tmp dir
mkdir -p ${tmpdir}/${bindir}/

## Copy to tmp from EOS
timestamp=$(eos ls ${indir})
xrdcp -r ${rootbase}/${indir}/${timestamp}/0000/ ${tmpdir}/${bindir}/

## Hadd on tmp 
hadd -O -k ${tmpdir}/${bindir}/${outfile} ${tmpdir}/${bindir}/${infiles}
rm -rf ${tmpdir}/${bindir}/${infiles}
rm -rf ${tmpdir}/${bindir}/failed/

## Copy back to EOS
eos mkdir -p ${outdir}
xrdcp -r ${tmpdir}/${bindir}/${outfile} ${rootbase}/${outdir}/
rm -rf ${tmpdir}/${bindir}/${outfile}
