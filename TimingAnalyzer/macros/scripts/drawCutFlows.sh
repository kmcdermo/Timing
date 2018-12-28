#!/bin/bash

## config
infilename=${1:-"${skimdir}/sr_hlt200.root"}
cutflowconfig=${2:-"${cutconfigdir}/one_at_a_time/signal_hlt200${inTextExt}"}
outfiletext=${3:-"sr_hlt200"}
dir=${4:-"ntuples_v4/checks_v3/cutflow"}

## first make plot
root -l -b -q plotCutFlows.C\(\"${infilename}\",\"${cutflowconfig}\",\"${outfiletext}\"\)

## make out dirs
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir "${fulldir}"

## copy everything
for ext in "${exts[@]}"
do
    cp ${outfiletext}.${ext} ${fulldir}
done

## Final message
echo "Finished PlottingCutFlows for:" ${infilename}
