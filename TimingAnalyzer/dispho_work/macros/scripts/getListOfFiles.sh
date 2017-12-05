#!/bin/bash

indir=${1}
tmptxt=${2}

eos ls ${indir}

grep ".root" > ${tmptxt}
rm tmp.txt
