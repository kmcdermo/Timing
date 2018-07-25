#!/bin/bash

## source first
source scripts/common_variables.sh

## config
cutflowconfig=${1:-"${cutconfigdir}/one_at_a_time/signal.${inTextExt}"}
outfiletext=${2:-"signal_skims"}

## produce slimmed skims
root -l -b -q runSignalSkimmer.C\(\"${cutflowconfig}\",\"${outfiletext}\"\)

## Final message
echo "Finished SignalSkimming"
