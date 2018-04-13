#!/bin/bash

cutconfig=${1:-"cut_config/signal_blinded.txt"}
plotconfig=${2:-"plot_config/met_vs_time.txt"}
pdname=${3:-"SinglePhoton"}
outfilename=${4:-"met_vs_time.root"}
dir=${5:-"plots2D"}

root -l -b -q runTreePlotter2D.C\(\"${cutconfig}\",\"${plotconfig}\",\"${pdname}\",\"${outfilename}\"\)

## copy out
topdir=/afs/cern.ch/user/k/kmcdermo/www/dispho
fulldir=${topdir}/${dir}

mkdir -p ${fulldir}
pushd ${topdir}
./makereadable.sh ${dir}
popd

cp ${outfilename} ${fulldir}
