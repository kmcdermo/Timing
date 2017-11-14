#!/bin/bash

source exportDataVars.sh

## input
label=${1}
era=${2}
version=${3}

## dirs needed
indir="${eosdir}/${dataset}/${label}_Run2017${era}-PromptReco-${version}"

## make tmp dir
mkdir -p ${tmpdir}/2017${era}/${version}

## Copy to tmp from EOS
timestamp=$(eos ls ${indir})
xrdcp -r ${rootbase}/${indir}/${timestamp}/0000/ ${tmpdir}/2017${era}/${version}
