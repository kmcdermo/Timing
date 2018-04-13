#!/bin/bash

cutconfig=${1:-"cut_config/signal_blinded_hlt.txt"}
plotconfig=${2:-"plot_config/phopt_0.txt"}
pdname=${3:-"SinglePhoton"}
scalearea=${4:-0}
outfiletext=${5:-"phopt_0"}
dir=${6:-"plots"}

## first make plot
root -l -b -q runTreePlotter.C\(\"${cutconfig}\",\"${plotconfig}\",\"${pdname}\",${scalearea},\"${outfiletext}\"\)

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
