#!/bin/bash

##################
## Source First ##
##################
source scripts/common_variables.sh

############
## Config ##
############

## command line inputs
outdirbase=${1:-"plots/ntuples_v4/checks_v4/era_plots"}
var=${2:-"TOF"}

for pho in "${phos[@]}"
do echo ${!pho} | while read -r index pho_label
    do
        ##########################
        ## Set plot config (1D) ##
        ##########################

	while IFS='' read -r line || [[ -n "${line}" ]]
	do
	    if   [[ "${line}" == "var="* ]]
	    then
		x_var=$( ReadConfig "${line}" )
		x_var="${x_var}_${index}"
	    elif [[ "${line}" == "title="* ]]
	    then
		title=$( ReadConfig "${line}" )
		title="${pho_label} ${title}"
	    elif [[ "${line}" == "bins="* ]]
	    then
		x_bins=$( ReadConfig "${line}" )
	    elif [[ "${line}" == "ytitle="* ]]
	    then
		ytitle=$( ReadConfig "${line}" )
	    fi
	done < "${fragdir}/${var}.${inTextExt}"
	
        ## loop over eta regions
	for eta in "${etas[@]}"
	do
	    #########################
	    ## make eta cut config ##
	    #########################

	    cut="tmp_cut_config.txt"
	    > "${cut}"

	    ## write common cut
	    common_cut="hltDiEle33MW&&!phoisOOT_0&&!phoisOOT_1&&phohasPixSeed_0&&phohasPixSeed_1"
	    eta_cut="phoisEB_${index}"
	    if [[ "${eta}" == "EB" ]]
	    then
		echo "common_cut=((${eta_cut})&&(${common_cut}))" >> "${cut}"
	    elif [[ "${eta}" == "EE" ]]
	    then
		echo "common_cut=(!(${eta_cut})&&(${common_cut}))" >> "${cut}"
	    elif [[ "${eta}" == "Full" ]]
	    then
		echo "common_cut=(${common_cut})" >> "${cut}"
	    else
		echo "How did this happen?? Did not choose a correct option for eta: ${eta} ... Exiting..."
		exit
	    fi
	    
            ## write the remainder of cuts
	    echo "data_cut=" >> "${cut}"
	    echo "bkgd_cut=" >> "${cut}"
	    echo "sign_cut=" >> "${cut}"
	    
	    ###########################
            ## make plot config (1D) ##
	    ###########################
	    
	    plot="tmp_pho_${index}_${var}_${eta}.${inTextExt}"
	    > "${plot}"
	    
	    echo "plot_title=${title} (${eta})" >> "${plot}"
	    echo "x_title=${title} (${eta})" >> "${plot}"
	    echo "x_var=${x_var}" >> "${plot}"
	    echo "x_bins=${x_bins}" >> "${plot}"
	    echo "y_title=${ytitle}" >> "${plot}"
	    
	    ######################################
	    ## determine which misc file to use ##
	    ######################################

	    misc=$(GetMisc ${input} ${plot})

	    ####################
	    ## loop over eras ##
	    ####################
	    for era in "${eras[@]}"
	    do
	        ################################
		## loop over inputs: Zee only ##
		################################
		for input in "${inputs[@]}"
		do echo ${!input} | while read -r label infile insigfile sel varwgtmap
		    do
                 	## outfile names
			outdir="${outdirbase}/${label}/${pho}/${eta}/${var}"
			outfile="${x_var}_${label}_${eta}_${era}"
			
			## run 1D plotter
			./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cut}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plot}" "${miscconfigdir}/${misc}.${inTextExt}" "${era}" "${outfile}" "${outdir}"
			
		    done ## read input
		done ## loop over inputs
	    done ## loop over eras

	    ## remove tmp files
	    rm "${cut}" "${plot}"

	done ## loop over etas
    done ## read pho
done ## loop over phos
