#!/bin/bash

var=${1:-"phopt_0"}
commoncut=${2:-"phopt_0>20.0"}
bkgdcut=${3:-"NONE"}
signcut=${4:-"NONE"}
datacut=${5:-"NONE"}
text=${6:-"overplot"}
nbinsx=${7:-100}
xlow=${8:-0}
xhigh=${9:-1000}
islogx=${10:-0}
islogy=${11:-1}
title=${12:-${var}}
xtitle=${13:-${var}}
ytitle=${14:-"Events"}
delim=${15:-"XXX"}
dir=${16:-"dispho"}

root -l -b -q runTreePlotter.C\(\"${var}\",\"${commoncut}\",\"${bkgdcut}\",\"${signcut}\",\"${datacut}\",\"${text}\",${nbinsx},${xlow},${xhigh},${islogx},${islogy},\"${title}\",\"${xtitle}\",\"${ytitle}\",\"${delim}\"\)

## copy out
topdir=/afs/cern.ch/user/k/kmcdermo/www
fulldir=${topdir}/${dir}

mkdir -p ${fulldir}
pushd ${topdir}
./makereadable.sh ${dir}
popd

cp ${text}.png ${fulldir}
