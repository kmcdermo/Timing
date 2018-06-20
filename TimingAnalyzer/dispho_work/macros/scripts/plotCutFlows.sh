#!/bin/bash

infilename=${1:-"skims/sr_hlt200.root"}
cutflowconfig=${2:-"cut_config/cuts_v3/one_at_a_time/signal_hlt200.txt"}
outfiletext=${3:-"sr_hlt200"}
dir=${4:-"plots/ntuples_v4/checks_v3/cutflow"}

## first make plot
root -l -b -q plotCutFlows.C\(\"${infilename}\",\"${cutflowconfig}\",\"${outfiletext}\"\)

## make out dirs
topdir=/afs/cern.ch/user/k/kmcdermo/www
fulldir=${topdir}/dispho/${dir}

## make them readable
mkdir -p ${fulldir}
pushd ${topdir}
./makereadable.sh ${fulldir}
popd

## copy everything
cp ${outfiletext}.png ${fulldir}

## Final message
echo "Finished PlottingCutFlows for:" ${infilename}
