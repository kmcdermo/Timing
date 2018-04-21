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

echo "Copying to ${fulldir}"
cp xfit_2D_lin.png yfit_2D_lin.png fit_projX_lin.png fit_projY_lin.png ${outfiletext}.root ${fulldir}
cp xfit_2D_log.png yfit_2D_log.png fit_projX_log.png fit_projY_log.png ${fulldir}
cp ${outfiletext}.root ${fulldir}
