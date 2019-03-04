#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
outdir=${1:-"madv2_v3/full_chain"}
is_blind=${2:-"true"}
use_obs=${3:-"false"}
save_meta_data=${4:-0}
do_cleanup=${5:-"true"}

## xs, ys bin boundaries to test (boundary, blind)
declare -a xs=("0 0" "0.5 0.5" "1 1" "1.5 1.5" "2 2" "3 3" "5 5")
declare -a ys=("50 50" "100 100" "150 150" "200 200" "300 300" "500 500" "1000 1000")

x_log="xs.${outTextExt}"
y_log="ys.${outTextExt}"

## Limit Config
outlimitdir="output"
outlimitplotdir="limits"
combinelogname="combine"
outcombname="AsymLim"

## Derived Config
fulldir="${topdir}/${disphodir}/${outdir}"

##################
## Compile Code ##
##################

echo "Compiling ahead of time"
./scripts/compile.sh

#############################
## Make All Analysis Tests ##
#############################

echo "Loop over all x,y boundary combos, run ABCD tests"

## Loop over all bin combos
for x in "${xs[@]}"
do
    echo "${x}" | while read -r xbin xblind
    do
	for y in "${ys[@]}"
	do
	    echo "${y}" | while read -r ybin yblind
	    do
	    	echo "Making analysis plots + limits for ${xbin},${ybin}"
		./scripts/makeAnalysisABCD.sh "${xbin}" "${xblind}" "${ybin}" "${yblind}" "${outlimitdir}" "${outlimitplotdir}" "${combinelogname}" "${outcombname}" "${outdir}/x_${xbin}_y_${ybin}" "${is_blind}" "${use_obs}" ${save_meta_data} "${do_cleanup}"

		## cleanup outputs
		if [[ "${do_cleanup}" == "true" ]]
		then
		    ./scripts/cleanup.sh
		fi
	    done
	done
    done
done

#####################################
## Make Final Limit Plot From Scan ##
#####################################

echo "Write x, y bins to tmp logs"

## write X first
> "${x_log}"
for x in "${xs[@]}"
do
    echo "${x}" >> "${x_log}"
done

## write Y second
> "${y_log}"
for y in "${ys[@]}"
do
    echo "${y}" >> "${y_log}"
done

echo "Making Final Limit Plot From ABCD Scan"
./scripts/makeLimitsFromScan.sh "${x_log}" "${y_log}" "${outlimitdir}" "${outlimitplotdir}" "${combinelogname}" "${outcombname}" "${outdir}" "${use_obs}" ${save_meta_data} "${do_cleanup}"

## cleanup if requested
if [[ "${do_cleanup}" == "true" ]]
then
    rm "${x_log}"
    rm "${y_log}"
    ./scripts/cleanup.sh
fi

####################
## Final Prep Dir ##
####################

echo "Final prep outdir"
PrepOutDir "${fulldir}"

###################
## Final Message ##
###################

echo "Finished full chain of analysis (ABCD)"
