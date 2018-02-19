#!/bin/bash

## first run plotter
cutconfig=${1:-"cut_config/signal_deg.txt"}
plotconfig=${2:-"plot_config/nvtx.txt"}
outfiletext=${3:-"puweights"}

root -l -b -q runTreePlotter.C\(\"${cutconfig}\",\"${plotconfig}\",\"${outfiletext}\"\)

## then run pu computation

root -l -b -q computePUWeights.C\(\"${outfiletext}\"\)

## move it somewhere SAFE

#xrdcp ${text}.root root://eoscms//store/user/kmcdermo/nTuples/
