#!/bin/bash

infilename=${1:-"test.root"}
insignalfilename=${2:-"signals.root"}
cutconfig=${3:-"cut_config/always_true.txt"}
plotsignals=${4:"plot_config/signals_to_plot.txt"}
plotconfig=${5:-"plot_config/phopt_0.txt"}
miscplotconfig=${6:-"plot_config/misc_blind.txt"}
outfiletext=${7:-"plots"}
dir=${8:-"plots/test"}

## first make plot
root -l -b -q runTreePlotter.C\(\"${infilename}\",\"${insignalfilename}\",\"${cutconfig}\",\"${plotsignals}\",\"${plotconfig}\",\"${miscplotconfig}\",\"${outfiletext}\"\)

## make out dirs
topdir=/afs/cern.ch/user/k/kmcdermo/www
fulldir=${topdir}/dispho/${dir}

## make them readable
mkdir -p ${fulldir}
pushd ${topdir}
./makereadable.sh ${fulldir}
popd

## copy everything
cp ${outfiletext}_log.png ${outfiletext}_lin.png ${outfiletext}.root ${outfiletext}_integrals.txt ${fulldir}
