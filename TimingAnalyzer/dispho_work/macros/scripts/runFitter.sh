#!/bin/bash

fitconfig=${1:-"fit_config/standard.txt"}
outfiletext=${2:-"fitresults"}
dir=${3:-"fits"}

root -b -q -l runFitter.C\(\"${fitconfig}\",\"${outfiletext}\"\)

## make out dirs
topdir=/afs/cern.ch/user/k/kmcdermo/www
fulldir=${topdir}/dispho/${dir}

## make them readable
mkdir -p ${fulldir}
pushd ${topdir}
./makereadable.sh ${fulldir}
popd

cp xfit_2D.png yfit_2D.png fit_projX.png fit_projY.png ${outfiletext}.root ${fulldir}
