#!/bin/bash

fitconfig=${1:-"fit_config/standard.txt"}
miscconfig=${2:-"plot_config/misc_fit.txt"}
outfiletext=${2:-"fitresults"}
dir=${3:-"fits"}

root -b -q -l runFitter.C\(\"${fitconfig}\",\"${miscconfig}\",\"${outfiletext}\"\)

## make out dirs
topdir=/afs/cern.ch/user/k/kmcdermo/www
fulldir=${topdir}/dispho/${dir}

## make them readable
mkdir -p ${fulldir}
pushd ${topdir}
./makereadable.sh ${fulldir}
popd

echo "Copying to ${fulldir}"
cp BkgdHist_2D.png BkgdHist_projX.png BkgdHist_projY.png ${fulldir}
cp SignHist_2D.png SignHist_projX.png SignHist_projY.png ${fulldir}
cp DataHist_2D.png DataHist_projX.png DataHist_projY.png ${fulldir}

cp *_xfit_2D_lin.png *_yfit_2D_lin.png *_fit_projX_lin.png *_fit_projY_lin.png ${fulldir}
cp *_xfit_2D_log.png *_yfit_2D_log.png *_fit_projX_log.png *_fit_projY_log.png ${fulldir}

cp ${outfiletext}.root ${fulldir}
