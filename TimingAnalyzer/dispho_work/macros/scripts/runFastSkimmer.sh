#!/bin/bash

cutconfig=${1:-"cut_config/signal_blinded_hlt.txt"}
pdname=${2:-"SinglePhoton"}
outfiletext=${3:-"phopt_0"}

## produce slimmed skim
root -l -b -q runFastSkimmer.C\(\"${cutconfig}\",\"${pdname}\",\"${outfiletext}\"\)
