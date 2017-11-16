#!/bin/bash

## input
label=${1}
era=${2}
version=${3}

## global vars
base="2017/DATA/singleph"
subera="2017${era}_${version}"
dataset="SinglePhoton"
eosbase="/store/user/${USER}"
rootbase="root://eoscms"

## filenames
infiles="dispho_*.root"
outfile="tree.root"

## directories needed
indir="${eosbase}/${dataset}/${label}_Run2017${era}-PromptReco-${version}"
tmpdir="/tmp/${USER}/${base}"
outdir="${eosbase}/nTuples/${base}/${subera}"

## make tmp dir
mkdir -p ${tmpdir}/${subera}

## Copy to tmp from EOS
timestamp=$(eos ls ${indir})
xrdcp -r ${rootbase}/${indir}/${timestamp}/0000/ ${tmpdir}/${subera}

## Hadd on tmp 
hadd -O -k ${tmpdir}/${subera}/${outfile} ${tmpdir}/${subera}/${infiles}
rm -rf ${tmpdir}/${subera}/${infiles}
rm -rf ${tmpdir}/${subera}/failed/

## Copy back to EOS
eos mkdir -p ${outdir}
xrdcp -r ${tmpdir}/${subera}/${outfile} ${rootbase}/${outdir}/
rm -rf ${tmpdir}/${subera}/${outfile}
