#!/bin/bash

infilename=${1:-"test.root"}
insignalfilename=${2:-"signals.root"}
cutconfig=${3:-"cut_config/signal_blinded.txt"}
plotconfig=${4:-"plot_config/met_vs_time.txt"}
outfiletext=${5:-"met_vs_time.root"}
dir=${6:-"plots2D"}

root -l -b -q runTreePlotter2D.C\(\"${infilename}\",\"${insignalfilename}\",\"${cutconfig}\",\"${plotconfig}\",\"${outfiletext}\"\)

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
