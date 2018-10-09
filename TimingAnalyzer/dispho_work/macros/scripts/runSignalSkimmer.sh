#!/bin/bash

## source first
source scripts/common_variables.sh

## config
cutflowconfig=${1:-"${cutconfigdir}/one_at_a_time/signal.${inTextExt}"}
inskimdir=${2:-"rereco_v4_metcorr"}
outfiletext=${3:-"signal_skims"}

## produce slimmed skims
root -l -b -q runSignalSkimmer.C\(\"${cutflowconfig}\",\"${inskimdir}\",\"${outfiletext}\"\)

## Final message
echo "Finished SignalSkimming"
