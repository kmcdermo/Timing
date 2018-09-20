#!/bin/bash

## source first
source scripts/common_variables.sh

textfile=${1}
plotconfig=${2}
outfiletext=${3}
dir=${4}

root -l -b -q test_macros/overplot_dataplots.C\(\"${textfile}\",\"${plotconfig}\",\"${outfiletext}\"\)

## make out dirs
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir ${fulldir}

## copy everything
for canvscale in "${canvscales[@]}"
do
    for ext in "${exts[@]}"
    do
	cp ${outfiletext}_${canvscale}.${ext} ${fulldir}
    done
done

## Final message
echo "Finished DataHistOverPlotting for plot:" ${plotconfig}
