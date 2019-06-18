#!/bin/bash

############
## Source ##
############

source scripts/common_variables.sh

############
## Config ##
############

region=${1:-"SR_ex1pho"}
time_split=${2:-"1.0"}
met_split=${3:-"100"}
savemetadata=${4:-0}
outdir=${5:-"madv2_v4p1/data_v_data"}
cleanup=${6:-"true"}

###########################
## Make temp Misc Config ##
###########################

misc_config="misc.${inTextExt}"
> "${misc_config}"
echo "skip_bkgd_mc=1" >> "${misc_config}"

###############
## Cut Flows ##
###############

## Time Cuts Left
time_cuts_L="time_cuts_L.${inTextExt}"
> "${time_cuts_L}"
echo "common_cut=" >> "${time_cuts_L}"
echo "data_cut=(phoweightedtimeLT120_0+phoweightedtimeLT120SHIFT_0)<${time_split}" >> "${time_cuts_L}"
echo "bkgd_cut=(phoweightedtimeLT120_0+phoweightedtimeLT120SHIFT_0+phoweightedtimeLT120SMEAR_0)<${time_split}" >> "${time_cuts_L}"
echo "sign_cut=(phoweightedtimeLT120_0+phoweightedtimeLT120SHIFT_0+phoweightedtimeLT120SMEAR_0)<${time_split}" >> "${time_cuts_L}"

## Time Cuts Right
time_cuts_R="time_cuts_R.${inTextExt}"
> "${time_cuts_R}"
echo "common_cut=" >> "${time_cuts_R}"
echo "data_cut=(phoweightedtimeLT120_0+phoweightedtimeLT120SHIFT_0)>=${time_split}" >> "${time_cuts_R}"
echo "bkgd_cut=(phoweightedtimeLT120_0+phoweightedtimeLT120SHIFT_0+phoweightedtimeLT120SMEAR_0)>=${time_split}" >> "${time_cuts_R}"
echo "sign_cut=(phoweightedtimeLT120_0+phoweightedtimeLT120SHIFT_0+phoweightedtimeLT120SMEAR_0)>=${time_split}" >> "${time_cuts_R}"

## MET Cuts Left
met_cuts_L="met_cuts_L.${inTextExt}"
> "${met_cuts_L}"
echo "common_cut=(t1pfMETpt)<${met_split}" >> "${met_cuts_L}"
echo "data_cut=" >> "${met_cuts_L}"
echo "bkgd_cut=" >> "${met_cuts_L}"
echo "sign_cut=" >> "${met_cuts_L}"

## MET Cuts Right
met_cuts_R="met_cuts_R.${inTextExt}"
> "${met_cuts_R}"
echo "common_cut=(t1pfMETpt)>=${met_split}" >> "${met_cuts_R}"
echo "data_cut=" >> "${met_cuts_R}"
echo "bkgd_cut=" >> "${met_cuts_R}"
echo "sign_cut=" >> "${met_cuts_R}"

#####################
## Run over Combos ##
#####################

## declare combos
declare -a combos=("${plotconfigdir}/phoweightedtimeLT120_0_final.${inTextExt} ${time_cuts_L} ${time_cuts_R} time" "${plotconfigdir}/met_final.${inTextExt} ${met_cuts_L} ${met_cuts_R} met")

## use only region from config
echo "${region}" | while read -r label infile insigfile sel
do    
    for combo in "${combos[@]}"
    do echo | while read -r plot_config cuts_L cuts_R group_label
	do
            ## declare out names
	    leftname="${label}_${group_label}_L"
	    rightname="${label}_${group_label}_R"
	    finalname="${label}_${group_label}"

            ## make left plot
	    echo "Making left 1D plot: ${group_label}"
	    ./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cuts_L}" "${plot_config}" "${misc_config}" "${MainEra}" ${save_meta_data} "${leftname}" "${outdir}"

            ## make right plot
	    echo "Making right 1D plot: ${group_label}"
	    ./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cuts_R}" "${plot_config}" "${misc_config}" "${MainEra}" ${save_meta_data} "${rightname}" "${outdir}"
	done
    done
done

##########################
## Cleanup if requested ##
##########################

if [[ "${do_cleanup}" == "true" ]]
then
    echo "Cleaning up..."
    rm "${misc_config}"
    rm "${met_cuts_L}" "${met_cuts_R}"
    rm "${time_cuts_L}" "${time_cuts_R}"
    ./scripts/cleanup.sh
fi

###################
## Final message ##
###################

echo "Finished data vs. data plots!"
