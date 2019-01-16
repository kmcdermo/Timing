#!/bin/bash

## source first 
source scripts/common_variables.sh

## config
cutflowconfig=${1:-"${cutconfigdir}/cuts_v1/one_at_a_time/orig_2phosCR/signal.${inTextExt}"}
pdname=${2:-"SinglePhoton"}
inskimdir=${3:-"madv2_v3"}
outfiletext=${4:-"sr"}
doskim=${5:-1}
sampleconfig=${6:-""}

## derived config
outfiletext_signal="signals_${outfiletext}"

## produce slimmed skim (Data + Bkgd)
nohup ./scripts/runFastSkimmer.sh "${cutflowconfig}" "${pdname}" "${inskimdir}" "${outfiletext}" ${doskim} "${sampleconfig}" >& "${outfiletext}.${outTextExt}" &

## produce slimmed skims (Signals)
nohup ./scripts/runSignalSkimmer.sh "${cutflowconfig}" "${inskimdir}" "${outfiletext_signal}" >& "${outfiletext_signal}.${outTextExt}" &
