#!/bin/bash

cutconfig=${1:-"cut_config/signal_blinded_hlt.txt"}
plotconfig=${2:-"plot_config/phopt_0.txt"}
scalearea=${3:-0}
outfiletext=${4:-"phopt_0"}
dir=${5:-"plots"}

./scripts/runTreePlotter.sh ${cutconfig} ${plotconfig} ${scalearea} ${outfiletext} ${dir}

root -l -b -q runDumpIntegrals.C\(\"${outfiletext}\"\)

## copy out
topdir=/afs/cern.ch/user/k/kmcdermo/www
fulldir=${topdir}/dispho/${dir}

cp ${outfiletext}_integrals.txt ${fulldir}
