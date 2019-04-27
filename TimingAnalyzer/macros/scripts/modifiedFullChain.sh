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
use_obs=${3:-"true"}
use_systematics=${4:-"false"}
save_meta_data=${5:-0}
do_cleanup=${6:-"true"}

## xy boundaries to plot: from optimized scan
declare -a xys=("0.5 0.5 200 200" "1.5 1.5 200 200")

## scan log --> set below
scan_log="abcd_categories.log"

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
for xy in "${xys[@]}"
do
    echo "${xy}" | while read -r xbin xblind ybin yblind
    do
	echo "Making analysis plots + limits for ${xbin},${ybin}"
	./scripts/makeAnalysisABCD.sh "${xbin}" "${xblind}" "${ybin}" "${yblind}" "${outdir}/x_${xbin}_y_${ybin}" "${is_blind}" "${use_obs}" "${use_systematics}" "false" ${save_meta_data} "${do_cleanup}"
	
	## cleanup outputs
	if [[ "${do_cleanup}" == "true" ]]
	then
	    ./scripts/cleanup.sh
	fi
    done
done

###########################
## Make Final Limit Plot ##
###########################

echo "Making log file for best dirs"
> "${scan_log}"

for lambda in 100 150 200 250 300 350 400
do
    for ctau in 1 10
    do 
	echo "GMSB_L${lambda}_CTau${ctau} ${fulldir}/x_0.5_y_200" >> "${scan_log}"
    done

    for ctau in 50 100 200 400 600 800 1000 1200 10000
    do
	echo "GMSB_L${lambda}_CTau${ctau} ${fulldir}/x_1.5_y_200" >> "${scan_log}"
    done
done

echo "Making Final Limit Plot"
./scripts/modifiedMakeLimitsFromScan.sh "${scan_log}" "${outdir}" "${use_obs}" ${save_meta_data} "${do_cleanup}"

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
