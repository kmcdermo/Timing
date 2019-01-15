#!/bin/bash

##################
## Source First ##
##################
source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
outdir=${1:-"madv2_v3/checks_v1/full_chain"}
docleanup=${2:-"true"}

## 2D plot config
plot="met_vs_time_box"
misc2D="empty"
outplot2Ddir="plots2D"
outtext="${plot}"

## Limit config
tmplimitdir="tmp"
doobs=0

######################
## Make Directories ##
######################

mkdir -p "${limitdir}/${inlimitdir}"

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
	./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${varwgtconfigdir}/empty.${inTextExt}" "${plotconfigdir}/${plot}.${inTextExt}" "${miscconfigdir}/${misc2D}.${inTextExt}" "${MainEra}" "${outtext}" "${outdir}/${outplot2Ddir}"

	## cp root file to limit directory
	cp "${outtext}.root" "${limitdir}/${tmplimitdir}"
    done
done

#########################
## Move Into Limit Dir ##
#########################

pushd "${limitdir}"

#########################
## Run Combine + Plots ##
#########################

./scripts/makeLimitsABCD.sh "${tmplimitdir}" "${outtext}.root" ${doobs} "${outdir}" "${docleanup}"

#########################
## Snap Back When Done ##
#########################

popd

###################
## Final Message ##
###################

echo "Finished making limits in full script (ABCD)"
