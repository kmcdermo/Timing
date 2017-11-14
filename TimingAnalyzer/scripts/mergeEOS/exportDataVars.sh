#!/bin/bash

## global vars
export base="2017/DATA/singleph"
export dataset="SinglePhoton"
export eosbase="/store/user/kmcdermo"
export rootbase="root://eoscms"

## filenames
export infiles="dispho_*.root"
export outfile="tree.root"

## directories needed
export tmpdir="/tmp/kmcdermo/${base}"
export outdir="${eosdir}/nTuples/${base}"
