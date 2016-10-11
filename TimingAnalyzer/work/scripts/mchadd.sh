#!/bin/sh

indir=$1
outdir=$2

for i in 1 2 3 4 5 6 7 8 9
do
    hadd ${indir}/${outdir}${i}/tree.root ${indir}/${outdir}${i}/tree_${i}*.root

    rm ${indir}/${outdir}${i}/tree_${i}*.root
done

#hadd ${dir}/tree.root ${dir}/tree?.root

#rm ${dir}/tree?.root