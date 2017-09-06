#!/bin/sh

indir=$1
outdir=$2

for i in 1 2 3 4 5 6 7 8 9
do 
    mkdir -p ${indir}/${outdir}${i}
    mv ${indir}/tree_${i}*.root ${indir}/${outdir}${i}
done