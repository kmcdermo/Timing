#!/bin/bash

# defaults are for running combine
fitconfig=${1:-"fit_config/ws_final.txt"} # full_model_bias.txt : for bias study with sig + bkg (toy data)
miscconfig=${2:-"misc_config/misc_fit.txt"} # misc_fit_model.txt : for bias study with sig + bkg (toy data)
outfiletext=${3:-"ws_final"}
outdir=${4:-"plots/ntuples_v4/checks_v2/fits"}

root -b -q -l runFitter.C\(\"${fitconfig}\",\"${miscconfig}\",\"${outfiletext}\"\)

## copy to combine --> comment out for bias study
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

## Final message
echo "Finished Fitting for fit: " ${fitconfig}
