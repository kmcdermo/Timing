#!/bin/bash

## input
label=${1}

## global vars
base="2017/MC/gjet-EM"
tune="DoubleEMEnriched_13TeV_pythia8"
eosbase="/store/user/kmcdermo"
rootbase="root://eoscms"

## filenames
infiles="dispho_*.root"
outfile="tree.root"

## directories needed
indir="${eosbase}/GJet_${tune}/${label}_GJet_${tune}"
tmpdir="/tmp/kmcdermo/${base}"
outdir="${eosbase}/nTuples/${base}"

## make tmp dir
mkdir -p ${tmpdir}/

## Copy to tmp from EOS
timestamp=$(eos ls ${indir})
xrdcp -r ${rootbase}/${indir}/${timestamp}/0000/ ${tmpdir}/

## Hadd on tmp 
hadd -O -k ${tmpdir}/${outfile} ${tmpdir}/${infiles}
rm -rf ${tmpdir}/${infiles}
rm -rf ${tmpdir}/failed/

## Copy back to EOS
eos mkdir -p ${outdir}
xrdcp -r ${tmpdir}/${outfile} ${rootbase}/${outdir}/
rm -rf ${tmpdir}/${outfile}
