#!/bin/bash

## input
label=${1}

## global vars
base="2017/MC/gmsb"
gridpt="L200TeV_CTau400cm_930"
eosbase="/store/user/${USER}"
rootbase="root://eoscms"

## filenames
infiles="dispho_*.root"
outfile="tree.root"

## directories needed
indir="${eosbase}/GMSB_${gridpt}/${label}_GMSB_{gridpt}"
tmpdir="/tmp/${USER}/${base}"
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
