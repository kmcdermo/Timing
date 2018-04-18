#!/bin/bash

cutconfig=${1:-"cut_config/signal_blinded_hlt.txt"}
pdname=${2:-"SinglePhoton"}
outfiletext=${3:-"phopt_0"}

## produce entry lists
root -l -b -q runEntryLister.C\(\"${cutconfig}\",\"${pdname}\",\"${outfiletext}\"\)
