#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Config
outfiletext=${1:-"met_vs_time"}
outdir=${2:-"madv2_v4p1/closure/v2"}

save_meta_data=${3:-0}
do_cleanup=${4:-"true"}

## Input Control Regions Used
declare -a CRs=(CR_GJets CR_QCD)

## plot configs
plot_config_zoom="${plotconfigdir}/met_vs_time_fine_zoom.${inTextExt}"
plot_config_full="${plotconfigdir}/met_vs_time_fine_full.${inTextExt}"

## misc config
misc_config="misc_closure_tests.${inTextExt}"
> "${misc_config}"
echo "skip_bkgd_mc=1" >> "${misc_config}"
echo "skip_signal=1"  >> "${misc_config}"

## output text files (used by all CR inputs)
outtextfile_zoom="${outfiletext}_zoom.${outTextExt}"
outtextfile_full="${outfiletext}_full.${outTextExt}"
declare -a outtextfiles=("${outtextfile_zoom}" "${outtextfile_full}")

for outtextfile in "${outtextfiles[@]}"
do
    > "${outtextfile}"
    echo "Control Region [Data],Time Boundary [ns],MET Boundary [GeV],obsA,obsB,obsC,obsD,c1{obsB/obsA},c2{obsD/obsA},predC{obsA*c1*c2},Percent Diff (1-predC/obsC),pullC{(obsC-predC)/sqrt(obsCunc^2+predCunc^2)},obsB/obsA,obsC/obsD,obsD/obsA,obsC/obsB" >> "${outtextfile}"
done

## combine group info
declare -a group_infos=("zoom ${plot_config_zoom} ${outtextfile_zoom}" "full ${plot_config_full} ${outtextfile_full}")

#######################################
## Main Loops Over Binnings + Inputs ##
#######################################

for group_info in "${group_infos[@]}"
do 
    echo "${group_info}" | while read -r group plot_config outtextfile
    do
	for CR in "${CRs[@]}"
	do
	    echo "${!CR}" | while read -r label infile insigfile sel
	    do
		## first make the 2D hist
		./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "NOSIGNALS.root" "${cutconfigdir}/always_true.${inTextExt}" "${plot_config}" "${misc_config}" "${MainEra}" ${save_meta_data} "${outfiletext}_${group}_${label}" "${outdir}"
		
		## run extractor
		./scripts/extractClosureUncertainty.sh "${label}" "${outfiletext}_${group}" "${outdir}"
	    done
	done
    done
done

##########################
## Final Copy to OutDir ##
##########################

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}"
PrepOutDir "${fulldir}"

## copy log files!
for outtextfile in "${outtextfiles[@]}"
do
    cp "${outtextfile}" "${fulldir}"
done

###################
## Final Cleanup ##
###################

if [[ "${do_cleanup}" == "true" ]]
then
    rm "${misc_config}"
    ./scripts/cleanup.sh
fi

###################
## Final Message ##
###################

echo "Finished Making Closure Tests!"
