#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
nominal=${1:-"SR"} # needs to be defined in common vars!
systunc=${2:-"PHO_SCALE_UP"} # needs to be defined in common vars!
systuncname=${3:-"phoscaleup"}
systunclabel=${4:-"Photon_Scale_Up"}
outdir=${5:-"madv2_v4/uncs_v3/pho_scale_up"}

save_meta_data=${6:-0}
do_cleanup=${7:-"true"}

####################
## Derived Config ##
####################

## full directory
fulldir="${topdir}/${disphodir}/${outdir}"

## nominal signal skim
IFS=" " read -r nom_label nom_infile nom_insigfile nom_sel <<< "${!nominal}"
nom_skim="${skimdir}/${nom_insigfile}.root"

## syst unc skim
IFS=" " read -r unc_label unc_infile unc_insigfile unc_sel <<< "${!systunc}"
unc_skim="${skimdir}/${unc_insigfile}.root"

####################################
## Configure which samples to use ##
####################################

echo "Making log file for which file to use"
scan_log="abcd_categories.log"
> "${scan_log}"

for lambda in 100 150 200 250 300 350 400
do
    for ctau in 0p001 0p1 10
    do
	echo "GMSB_L${lambda}_CTau${ctau} ${fulldir} x_0.5_y_200" >> "${scan_log}"
    done

    for ctau in 200 400 600 800 1000 1200 10000
    do
	echo "GMSB_L${lambda}_CTau${ctau} ${fulldir} x_1.5_y_200" >> "${scan_log}"
    done
done

## xy boundaries to plot: from optimized scan
declare -a xys=("0.5 0.5 200 200" "1.5 1.5 200 200")

##################
## Compile Code ##
##################

echo "Compiling ahead of time"
./scripts/compile.sh

###############
## 1-D plots ##
###############

echo "Making 1D plot comparisons"
./scripts/compareSignalUncs.sh "${scan_log}" "${nom_skim}" "${unc_skim}" "${systuncname}" "${systunclabel}" "${outdir}/plots1D"

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
    done
done

#########################
## Extract final diffs ##
#########################

echo "Extract Differences"
./scripts/extractSignalDiffs.sh "${scan_log}" "${systuncname}" "${outdir}/diffs"

###################
## Final Cleanup ##
###################

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
