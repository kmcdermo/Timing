#!/bin/bash

##################
## Source First ##
##################
source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
outdir=${1:-"ntuples_v4/full_chain"}
savemetadata=${2:-0}
docleanup=${3:-"true"}

## Tmp Info
tmpdir="tmp"

## 2D plot config
plot="met_vs_time"
misc2D="empty"
outplot2Ddir="plots2D"

## Fitter config
infitdir="input"
fit="ws_final"
fitconfig="${tmpdir}/${fit}.${inTextExt}"
misc_fit="misc_fit"
outfitdir="fits"

## Limit config
inlimitdir="input"
doobs=0

######################
## Make Directories ##
######################

mkdir -p "${tmpdir}"
mkdir -p "${infitdir}"
mkdir -p "${limitdir}/${inlimitdir}"

################################
## Clean Slate For Tmp Config ##
################################

> "${fitconfig}"

#########################
## Make 2D Input Plots ##
#########################

for input in "${inputs[@]}"
do 
    echo ${!input} | while read -r label infile insigfile sel varwgtmap
    do
	## tmp out name
	outtext="${plot}_${label}"

	## make plot
	./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plotconfigdir}/${plot}.${inTextExt}" "${miscconfigdir}/${misc2D}.${inTextExt}" "${MainEra}" ${savemetadata} "${outtext}" "${outdir}/${outplot2Ddir}"

	## cp root file to local directory
	cp "${outtext}.root" "${infitdir}"

	## write to tmp config file
	echo "${input}_in=${infitdir}/${outtext}.root" >> "${fitconfig}"
    done
done

##################################
## Finish details on fit config ##
##################################

echo "${plotconfigdir}=${plotconfigdir}/${plot}.${inTextExt}" >> "${fitconfig}"
echo "scale_range_low=-10" >> "${fitconfig}"
echo "scale_range_high=10" >> "${fitconfig}"
echo "make_ws=1" >> "${fitconfig}"
echo "era=${MainEra}" >> "${fitconfig}"

#################################
## Run Fitter Over Input Plots ##
#################################

./scripts/runFitter.sh "${fitconfig}" "${miscconfigdir}/${misc_fit}.${inTextExt}" ${savemetadata} "${fit}" "${outdir}/${outfitdir}"

#####################################
## Copy To Local Combine Directory ##
#####################################

cp "${fit}.root" "${limitdir}/${inlimitdir}"

#########################
## Move Into Limit Dir ##
#########################

pushd "${limitdir}"

#########################
## Run Combine + Plots ##
#########################

./scripts/makeLimits.sh "${inlimitdir}" "${fit}.root" ${doobs} "${outdir}" ${savemetadata} "${docleanup}"

#########################
## Snap Back When Done ##
#########################

popd

###########################
## Clean Up If Requested ##
###########################

if [[ "${docleanup}" == "true" ]]; then
    echo "Cleaning up tmp dirs: ${infitdir}, ${tmpdir}"
    rm -r "${infitdir}"
    rm -r "${tmpdir}"
fi

###################
## Final Message ##
###################

echo "Finished making limits in full script"
