#!/bin/bash

infilename=${1:-"test.root"}
insignalfilename=${2:-"signals.root"}
cutconfig=${3:-"cut_config/always_true.txt"}
plotconfig=${4:-"plot_config/phopt_0.txt"}
scalearea=${5:-0}
outfiletext=${6:-"plots"}
dir=${7:-"plots/test"}

## first make plot
root -l -b -q runTreePlotter.C\(\"${infilename}\",\"${insignalfilename}\",\"${cutconfig}\",\"${plotconfig}\",${scalearea},\"${outfiletext}\"\)

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
