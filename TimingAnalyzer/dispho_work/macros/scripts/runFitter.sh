#!/bin/bash

## source first
source scripts/common_variables.sh

## config: defaults are for running combine
fitconfig=${1:-"${fitconfigdir}/ws_final.${inTextExt}"} # full_model_bias.${inTextExt} : for bias study with sig + bkg (toy data)
miscconfig=${2:-"${miscconfigdir}/misc_fit.${inTextExt}"} # misc_fit_model.${inTextExt} : for bias study with sig + bkg (toy data)
outfiletext=${3:-"ws_final"}
outdir=${4:-"plots/ntuples_v4/checks_v3/fits"}

## make array of dimensions
declare -a dims=("2D" "projX" "projY")

## run macro
root -b -q -l runFitter.C\(\"${fitconfig}\",\"${miscconfig}\",\"${outfiletext}\"\)

## make out dirs
fulldir=${topdir}/${disphodir}/${outdir}
PrepOutDir ${fulldir}

for dim in "${dims[@]}"
do
    PrepOutDir ${fulldir}/${dim}
done

echo "Copying to ${fulldir}"
for sample in BkgdHist *SignHist DataHist *Significance
do
    for ext in "${exts[@]}"
    do
	for dim in "${dims[@]}"
	do
	    if [[ "${dim}" == "2D" ]]; then
		cp ${sample}_${dim}.${ext} ${fulldir}/${dim}
	    else
		for canvscale in "${canvscales[@]}"
		do
		    cp ${sample}_${dim}_${canvscale}.${ext} ${fulldir}/${dim}
		done
	    fi
	done
    done
done

for fit in xfit_2D yfit_2D fit_projX fit_projY
do 
    for canvscale in "${canvscales[@]}"
    do
	for ext in "${exts[@]}"
	do
	    cp *_${fit}_${canvscale}.${ext} ${fulldir}
	done
    done
done
cp ${outfiletext}.root ${fulldir}

## Final message
echo "Finished Fitting for fit:" ${fitconfig}
