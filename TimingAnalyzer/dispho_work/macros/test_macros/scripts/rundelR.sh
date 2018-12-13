#!/bin/bash

source scripts/common_variables.sh

## config
infilename=${1:-"skims/v2/orig_2phosCR/gjets.root"}
outfiletext=${2:-"gjets"}
dir=${3:-"madv2_v2/test/delR_checks_v2"}

## first make plot
root -l -b -q test_macros/delRplots.C\(\"${infilename}\",\"${outfiletext}\"\)

## make out dirs
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir ${fulldir}

for hist in delR delR_zoom met delE delseedE delseedE_zoom delseedtime seedtime0 seedtime1 delweightedtime weightedtime0 weightedtime1
do
    cp ${hist}_${outfiletext}.png ${fulldir}
done
