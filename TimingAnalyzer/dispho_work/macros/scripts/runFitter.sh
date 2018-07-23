#!/bin/bash

# defaults are for running combine
fitconfig=${1:-"fit_config/ws_final.txt"} # full_model_bias.txt : for bias study with sig + bkg (toy data)
miscconfig=${2:-"misc_config/misc_fit.txt"} # misc_fit_model.txt : for bias study with sig + bkg (toy data)
outfiletext=${3:-"ws_final"}
outdir=${4:-"plots/ntuples_v4/checks_v3/fits"}

root -b -q -l runFitter.C\(\"${fitconfig}\",\"${miscconfig}\",\"${outfiletext}\"\)

## make out dirs
topdir=/afs/cern.ch/user/k/kmcdermo/www
fulldir=${topdir}/dispho/${outdir}

## make them readable
mkdir -p ${fulldir}
pushd ${topdir}
./makereadable.sh ${fulldir}
popd

echo "Copying to ${fulldir}"
for sample in Bkgd Sign Data
do
    for dim in 2D projX projY
    do
	for ext in png pdf eps
	do
	    cp ${sample}Hist_${dim}.${ext} ${fulldir}
	done
    done
done

for fit in xfit_2D yfit_2D fit_projX fit_projY
do 
    for canvscale in log lin
    do
	for ext in png pdf eps
	do
	    cp *_${fit}_${canvscale}.${ext}
	done
    done
done
cp ${outfiletext}.root ${fulldir}

## Final message
echo "Finished Fitting for fit:" ${fitconfig}
