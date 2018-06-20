#!/bin/bash

cutflowconfig=${1:-"cut_config/one_at_a_time/signal_idT_hlt_nJet3pt30_ht400jetpt30_sumEtjetpt30phopt0_hltreco.txt"}
outfiletext=${2:-"signal_skims"}

## produce slimmed skims
root -l -b -q runSignalSkimmer.C\(\"${cutflowconfig}\",\"${outfiletext}\"\)

## Final message
echo "Finished SignalSkimming"
