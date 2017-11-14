#!/bin/bash

## input
label=${1}
bin=${2}

## global vars
base="2017/MC/qcd"
tune="EMEnriched_TuneCUETP8M1_13TeV_pythia8"
eosbase="/store/user/kmcdermo"
rootbase="root://eoscms"

## filenames
infiles="dispho_*.root"
outfile="tree.root"

## directories needed
indir="${eosdir}/QCD_Pt-${bin}_${tune}/${label}_QCD_Pt-${bin}_${tune}"
tmpdir="/tmp/kmcdermo/${base}"
outdir="${eosdir}/nTuples/${base}/Pt-${bin}"

## make tmp dir
mkdir -p ${tmpdir}/Pt-${bin}/

## Copy to tmp from EOS
timestamp=$(eos ls ${indir})
xrdcp -r ${rootbase}/${indir}/${timestamp}/0000/ ${tmpdir}/Pt-${bin}/

## Hadd on tmp 
hadd -O -k ${tmpdir}/Pt-${bin}/${outfile} ${tmpdir}/Pt-${bin}/${infiles}
rm -rf ${tmpdir}/Pt-${bin}/${infiles}
rm -rf ${tmpdir}/Pt-${bin}/failed/

## Copy back to EOS
eos mkdir -p ${outdir}
xrdcp -r ${tmpdir}/Pt-${bin}/${outfile} ${rootbase}/${outdir}/
rm -rf ${tmpdir}/Pt-${bin}/${outfile}
