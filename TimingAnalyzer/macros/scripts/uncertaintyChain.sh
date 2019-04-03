#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
systnom=${1:-"SYST_NOM"} # needs to be defined in common vars!
systunc=${2:-"PHO_SC_UP"} # needs to be defined in common vars!
systuncname=${2:-"phoscaleup"}
systunclabel=${4:-"Photon_Scale_Up"}
outdir=${5:-"madv2_v4/uncs/pho_scale_up"}

save_meta_data=${6:-0}
do_cleanup=${7:-"true"}

## xy boundaries to plot: from optimized scan
declare -a xys=("1.5 1.5 500 500" "0.5 0.5 200 200" "1.5 1.5 200 200")

## Derived Config
fulldir="${topdir}/${disphodir}/${outdir}"

## nominal signal skim
echo "${!systnom}" | while read -r label infile insigfile sel
do    
    nom_skim="${skimdir}/${insigfile}.root"
done

## syst unc skim
echo "${!systunc}" | while read -r label infile insigfile sel
do    
    unc_skim="${skimdir}/${insigfile}.root"
done

##################
## Compile Code ##
##################

echo "Compiling ahead of time"
./scripts/compile.sh

###############
## 1-D plots ##
###############

echo "Making 1D plot comparisons"
./scripts/compareSigUncs.sh "${nom_skim}" "${unc_skim}" "${systuncname}" "${systunclabel}" "${outdir}/plots1D"

#########################
## Make All ABCD Plots ##
#########################

echo "Make all ABCD Plots"

## Loop over all bin combos
for xy in "${xys[@]}"
do
    echo "${xy}" | while read -r xbin xblind ybin yblind
    do
	echo "Making plot for ${xbin},${ybin}"
	
	## label for each root file
	plotlabel="x_${xbin}_y_${ybin}"

	## run plotter
	./scripts/makePlotsForUnc.sh "${nom_skim}" "${unc_skim}" "${xbin}" "${xblind}" "${ybin}" "${yblind}" "${plotlabel}" "${outdir}" ${save_meta_data} "${do_cleanup}"
	
	## cleanup outputs
	if [[ "${do_cleanup}" == "true" ]]
	then
	    ./scripts/cleanup.sh
	fi
    done
done

#########################
## Extract final diffs ##
#########################

echo "Making log file for which file to use"
scan_log="abcd_categories.log"
> "${scan_log}"

for lambda in 100 200 300
do
    for ctau in 10
    do 
	echo "GMSB_L${lambda}_CTau${ctau} ${fulldir} x_0.5_y_200" >> "${scan_log}"
    done

    for ctau in 200 1000
    do
	echo "GMSB_L${lambda}_CTau${ctau} ${fulldir} x_1.5_y_200" >> "${scan_log}"
    done
done

echo "Extract Differences"
./scripts/extractSigDiffs.sh "${scan_log}" "${systuncname}" "${outdir}/diffs"

## cleanup if requested
if [[ "${do_cleanup}" == "true" ]]
then
    rm "${scan_log}"
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

echo "Finished full chain of uncertainty analysis"
