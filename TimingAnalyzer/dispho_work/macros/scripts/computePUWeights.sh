#!/bin/bash

## first run plotter
var=${1:-"nvtx"}
commoncut=${2:-"(phopt_0>70.0&&phoID_0>=2)&&hltSignal"}
text=${3:-"puweights"}
nbinsx=${4:-120}
xlow=${5:-0}
xhigh=${6:-120}
islogx=${7:-0}
islogy=${8:-1}
title=${9:-${var}}
xtitle=${10:-${var}}
ytitle=${11:-"Events"}

root -l -b -q runTreePlotter.C\(\"${var}\",\"${commoncut}\",\"${text}\",${nbinsx},${xlow},${xhigh},${islogx},${islogy},\"${title}\",\"${xtitle}\",\"${ytitle}\"\)

## then run pu computation

root -l -b -q computePUWeights.C\(\"${text}\"\)

## move it somewhere SAFE

xrdcp ${text}.root root://eoscms//store/user/kmcdermo/nTuples/
