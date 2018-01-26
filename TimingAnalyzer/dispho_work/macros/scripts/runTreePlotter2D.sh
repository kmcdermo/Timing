#!/bin/bash

cutconfig=${1:-"cut_config/signal.txt"}
plotconfig=${2:-"plot_config/met_vs_time.txt"}
outfilename=${3:-"met_vs_time.root"}
dir=${4:-"dispho/2Dplots"}

root -l -b -q runTreePlotter2D.C\(\"${cutconfig}\",\"${plotconfig}\",\"${outfilename}\"\)

## copy out
topdir=/afs/cern.ch/user/k/kmcdermo/www
fulldir=${topdir}/${dir}

mkdir -p ${fulldir}
pushd ${topdir}
./makereadable.sh ${dir}
popd

cp ${text}.png ${fulldir}
