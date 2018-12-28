#!/bin/bash

## source first 
source scripts/common_variables.sh

## config
cutflowconfig=${1:-"${cutconfigdir}/cuts_v2/one_at_a_time/control_qcd.${inTextExt}"}
pdname=${2:-"SinglePhoton"}
inskimdir=${3:-"rereco_v4_metcorr"}
outfiletext=${4:-"skim"}
doskim=${5:-1}
sampleconfig=${6:-""}

## produce slimmed skim
root -l -b -q runFastSkimmer.C\(\"${cutflowconfig}\",\"${pdname}\",\"${inskimdir}\",\"${outfiletext}\",${doskim},\"${sampleconfig}\"\)

## Final message
echo "Finished FastSkimming"
