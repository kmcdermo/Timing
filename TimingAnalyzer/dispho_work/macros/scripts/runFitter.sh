#!/bin/bash

fitconfig=${1:-"fit_config/ws_final.txt"}
miscconfig=${2:-"misc_config/misc_fit.txt"}
outfiletext=${3:-"ws_final"}
outdir=${4:-"plots/ntuples_v4/fits"}

root -b -q -l runFitter.C\(\"${fitconfig}\",\"${miscconfig}\",\"${outfiletext}\"\)

## copy to combine
combinedir="combine/input"
mkdir -p ${combinedir}
cp ${outfiletext}.root ${combinedir} 

## make out dirs
topdir=/afs/cern.ch/user/k/kmcdermo/www
fulldir=${topdir}/dispho/${outdir}

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
