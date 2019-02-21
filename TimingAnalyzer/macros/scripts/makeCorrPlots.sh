#!/bin/bash

##################
## Source First ##
##################
source scripts/common_variables.sh

############
## Config ##
############

## command line inputs
outdirbase=${1:-"ntuples_v4/checks_v4/era_plots"}
savemetadata=${2:-0}
var=${3:-"TOF"}

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
	    elif [[ "${line}" == "unit="* ]]
	    then
		unit=$( ReadConfig "${line}" )
	    elif [[ "${line}" == "bins="* ]]
	    then
		x_bins=$( ReadConfig "${line}" )
	    elif [[ "${line}" == "ytitle="* ]]
	    then
		ytitle=$( ReadConfig "${line}" )
	    fi
	done < "${fragdir}/${var}.${inTextExt}"
	
	## add units to title
	if [[ "${unit}" != "" ]]
	then
	    title+=" ${unit}"
	fi
	
        ## loop over eta regions
	for eta in "${etas[@]}"
	do
	    #########################
	    ## make eta cut config ##
	    #########################

	    cut="tmp_cut_config.txt"
	    > "${cut}"

	    ## common cuts
	    common_cut="(phohasPixSeed_0&&phohasPixSeed_1)"
	    
	    ## eta cuts
	    eta_cut_base="phoisEB_${index}"
	    if [[ "${eta}" == "Full" ]]
	    then
		eta_cut="(1)"
	    elif [[ "${eta}" == "EB" ]]
	    then
		    eta_cut="(${eta_cut_base})"
	    elif [[ "${eta}" == "EE" ]]
	    then
		eta_cut="(!${eta_cut_base})"
	    else
		echo "How did this happen?? Did not choose a correct option for eta: ${eta} ... Exiting..."
		exit
	    fi
	    
	    ## write the remainder of cuts
	    echo "common_cut=${common_cut}&&${eta_cut}" >> "${cut}"
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

	    misc=$( GetMisc ${input} ${plot} )

	    ####################
	    ## loop over eras ##
	    ####################
	    for era in "${eras[@]}"
	    do
	        ################################
		## loop over inputs: Zee only ##
		################################
		for input in "${inputs[@]}"
		do echo ${!input} | while read -r label infile insigfile sel
		    do
                 	## outfile names
			outdir="${outdirbase}/${label}/${pho}/${eta}/${var}"
			outfile="${x_var}_${label}_${eta}_${era}"
			
			## run 1D plotter
			./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cut}" "${plot}" "${miscconfigdir}/${misc}.${inTextExt}" "${era}" ${savemetadata} "${outfile}" "${outdir}"
			
		    done ## read input
		done ## loop over inputs
	    done ## loop over eras

	    ## remove tmp files
	    rm "${cut}" "${plot}"

	done ## loop over etas
    done ## read pho
done ## loop over phos
