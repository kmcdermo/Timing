#!/bin/bash

var=${1:-"phopt_0"}
text=${2:-"overplot"}
xbinsx=${3:-100}
xlow=${4:-0}
xhigh=${5:-100}
title=${6:-${var}}
xtitle=${7:-${var}}
ytitle=${8:-"Events"}

root -l TreePlotter.C\(\"${var}\",\"${text}\",nbinsx,xlow,xhigh,\"${title}\",\"${xtitle}\",\"${ytitle}\"\)
