#!/bin/bash

cutconfig=${1:-"cut_config/signal_blinded.txt"}
plotconfig=${2:-"plot_config/met_vs_time.txt"}
pdname=${3:-"SinglePhoton"}
outfiletext=${4:-"met_vs_time.root"}
dir=${5:-"plots2D"}

root -l -b -q runTreePlotter2D.C\(\"${cutconfig}\",\"${plotconfig}\",\"${pdname}\",\"${outfiletext}\"\)

## make out dirs
topdir=/afs/cern.ch/user/k/kmcdermo/www
fulldir=${topdir}/dispho/${dir}

## make them readable
mkdir -p ${fulldir}
pushd ${topdir}
./makereadable.sh ${fulldir}
popd

## copy everything
cp ${outfiletext}.root ${fulldir}
