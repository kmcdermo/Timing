#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"madv2_v3/full_chain"}
is_blind=${2:-"true"}
use_obs=${3:-"false"}
save_meta_data=${4:-0}
do_cleanup=${5:-"true"}

## for safety
echo "Compiling ahead of time"
./scripts/compile.sh

## make signal efficiencies
echo "Making Signal Efficiencies"
./scripts/makeSignalEffs.sh "${outdir}/sig_effs"

## make Data/MC plots (no weights yet)
echo "Making 1D Data/MC plots with no weights"
./scripts/make1Dplots.sh "${outdir}/data_over_mc" "${reducedplotlist}" "false" ${save_meta_data}

## xs, ys
declare -a xs=("1.5 1.5")
declare -a ys=("150 150")

## loop over all bin combos
for x in "${xs[@]}"
do
    echo "${x}" | while read -r xbin xblind
    do
	for y in "${ys[@]}"
	do
	    echo "${y}" | while read -r ybin yblind
	    do
	    	echo "Making analysis plots + limits for ${xbin},${ybin}"
		./scripts/makeAnalysisABCD.sh "${xbin}" "${xblind}" "${ybin}" "${yblind}" "${outdir}/x_${xbin}_y_${ybin}" "${is_blind}" "${use_obs}" ${save_meta_data} "${do_cleanup}"

		## cleanup outputs
		if [[ "${do_cleanup}" == "true" ]]
		then
		    ./scripts/cleanup.sh
		fi
	    done
	done
    done
done

## final prep dir
echo "Final prep outdir"
PrepOutDir "${topdir}/${disphodir}/${outdir}"

## all done
echo "Finished full chain of analysis (ABCD)"
