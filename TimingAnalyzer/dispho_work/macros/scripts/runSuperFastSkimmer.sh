#!/bin/bash

## source first
source scripts/common_variables.sh

## config
cutflowconfig=${1:-"${cutconfigdir}/one_at_a_time/signal.${inTextExt}"}
infilename=${2:-"${skimdir}/signals.root"}
issignalfile=${3:-1}
outfiletext=${4:-"signal_skims"}

## produce slimmed skims
root -l -b -q runSuperFastSkimmer.C\(\"${cutflowconfig}\",\"${infilename}\",${issignalfile},\"${outfiletext}\"\)

## Final message
echo "Finished SuperFastSkimming"
