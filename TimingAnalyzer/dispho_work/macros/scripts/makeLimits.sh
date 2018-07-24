#!/bin/bash

###################
## Configuration ##
###################

## Command Line Input
outdir=${1:-"plots/ntuples_v4/full_chain"}

## Tmp Info
tmpdir="tmp"

## 2D plot config
plot="met_vs_time"
misc2D="empty"
outplot2Ddir="plots2D"

## 2D plot inputs
CR_GJets="gjets signals_gjets always_true gjets_phopt_0_map cr_gjets_DEG"
CR_QCD="qcd signals_qcd cuts_v3/invertiso0_v0 qcd_phopt_0_map cr_qcd_DEG"
SR="sr signals_sr always_true empty sr_SPH"
declare -a inputs=(CR_GJets CR_QCD SR)

## Fitter config
infitdir="input"
fit="ws_final"
fitconfig="${tmpdir}/${fit}.txt"
misc_fit="misc_fit"
outfitdir="fits"

## Combine config
limitdir="limits"
inlimitdir="input"
outcombname="AsymLim"
outlimitdir="output"

## Limit plot config
doobs=0
limit="limits2D"
outlimit1D="limit1D"
outlimit1D="limit2D"
outlimitplotdir="limits"

######################
## Make Directories ##
######################

mkdir -p "${tmpdir}"
mkdir -p "${infitdir}"
mkdir -p "${limitdir}/${inlimitdir}"

#########################
## Make 2D Input Plots ##
#########################

for input in "${inputs[@]}"
do 
    echo ${!input} | while read -r infile insigfile sel varwgtmap label
    do
	## tmp out name
	outtext="${plot}_${label}"

	## make plot
	./scripts/runTreePlotter2D.sh "skims/${infile}.root" "skims/${insigfile}.root" "cut_config/${sel}.txt" "vargwgt_config/${varwgmap}.txt" "plot_config/${plot}.txt" "misc_config/${misc2D}.txt" "${outtext}" "${outdir}/${outplot2Ddir}"

	## cp root file to local directory
	cp "${outtext}.root" "${infitdir}"

	## write to tmp config file
	echo "${input}_in=${infitdir}/${outtext}.root" >> "${fitconfig}"
    done
done

##################################
## Finish details on fit config ##
##################################

echo "plot_config=plot_config/${plot}.txt" >> "${fitconfig}"
echo "scale_range_low=-10" >> "${fitconfig}"
echo "scale_range_high=10" >> "${fitconfig}"
echo "make_ws=1" >> "${fitconfig}"

#################################
## Run Fitter Over Input Plots ##
#################################

./scripts/runFitter.sh "${fitconfig}" "misc_config/${misc_fit}.txt" "${fit}" "${outdir}/${outfitdir}"

###################
## Remove tmpdir ##
###################

rm -r "${tmpdir}"

#####################################
## Copy To Local Combine Directory ##
#####################################

cp "${fit}.root" "${limitdir}/${inlimitdir}"

#####################
## Move Into Limit ##
#####################

pushd "${limitdir}"

###############################################
## Extract Limits From Fitter : Run Combine! ##
###############################################

./scripts/extractResults.sh "${inlimitdir}" "${fit}.root" "${outcombname}" "${outlimitdir}" 

#########################
## Make 1D Limit Plots ##
#########################

./scriplts/runLimits1D.sh "${outlimitdir}" "${outcombname}" ${doobs} "${outlimit1D}" "${outdir}/${outlimitplotdir}"

#########################
## Make 2D Limit Plots ##
#########################

./scriplts/runLimits1D.sh "${outlimitdir}" "${outcombname}" "limit_config/${limit}.txt" "${outlimit2D}" "${outdir}/${outlimitplotdir}"

#########################
## Snap Back When Done ##
#########################

popd

###################
## Final Message ##
###################

echo "Finished making limits in full script"
