#!/bin/bash

## source first 
source scripts/common_variables.sh

## config
cutflowconfig=${1:-"${cutconfigdir}/cuts_v2/one_at_a_time/control_qcd.${inTextExt}"}
pdname=${2:-"SinglePhoton"}
outfiletext=${3:-"skim"}
doskim=${4:-1}
sampleconfig=${5:-""}

## produce slimmed skim
root -l -b -q runFastSkimmer.C\(\"${cutflowconfig}\",\"${pdname}\",\"${outfiletext}\",${doskim},\"${sampleconfig}\"\)

## Final message
echo "Finished FastSkimming"
