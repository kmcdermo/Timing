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
usecorr=${2:-"false"}

## other info
fragdir="plot_config/fragments"}

## etas
declare -a etas=("EB" "EE" "Full")

## vars
declare -a vars=("E" "pt" "eta" "time" "nvtx")

## logx vars
declare -a logx_vars=("E" "pt")

## phos
pho0="0 Leading"
pho1="1 Subleading"
declare -a phos=("pho0" "pho1")

## function to read config
function ReadConfig ()
{
    local line=${1}
    echo "${line}" | cut -d "=" -f 2
}

## function to say if logx
function CheckLogX ()
{
    local var=${1}
    local result="false"

    for logx_var in "${logx_vars[@]}"
    do
	if [[ "${var}" == "${logx_var}" ]]
	then
	    result="true"
	    break
	fi
    done
    
    echo "${result}"
}

## make 1D and 2D plots
for pho in "${phos[@]}"
do echo ${!pho} | while read -r index pho_label
    do
	for var in "${vars[@]}"
	do 
	    ## read in plot info (1D)
	    while IFS='' read -r line || [[ -n "${line}" ]]
	    do
		if   [[ "${line}" == "var="* ]]
		then
		    x_var=$( ReadConfig "${line}" )
		elif [[ "${line}" == "title="* ]]
		then
		    title=$( ReadConfig "${line}" )
		elif [[ "${line}" == "bins="* ]]
		then
		    x_bins=$( ReadConfig "${line}" )
		fi
	    done < "${fragdir}/${var}.${inTextExt}"

	    ## add in photon index
	    if [[ "${var}" != "nvtx" ]] 
	    then
		title="${pho_label} ${title}"
		x_var="${x_var}_${index}"
	    fi

	    ## read in time plot info (2D)
	    while IFS='' read -r line || [[ -n "${line}" ]]
	    do
		if   [[ "${line}" == "var="* ]]
		then
		    time_var=$( ReadConfig "${line}" )
		    time_var="${time_var}_${index}"
		elif [[ "${line}" == "title="* ]]
		then
		    time_title=$( ReadConfig "${line}" )
		    time_title="${pho_label} ${time_title}"
		elif [[ "${line}" == "bins="* ]]
		then
		    time_bins=$( ReadConfig "${line}" )
		fi
	    done < "${fragdir}/time.${inTextExt}"
	    
	    ## set corrections
	    std_corr="+phoseedTOF_${index}"
	    data_corr="${std_corr}"
	    mc_corr="${std_corr}"

	    if [[ "${usecorr}" == "true" ]]
	    then
		data_corr="+phoseedtimeSHIFT_${index}" 
		mc_corr="+phoseedtimeSHIFT_${index}+phoseedtimeSMEAR_${index}"
	    fi

	    ## loop over eta regions
	    for eta in "${etas[@]}"
	    do
		## make eta cut config
		cut="tmp_cut_config.txt"
		> "${cut}"

		## write common cut
		eta_cut="phoisEB_${index}"
		if [[ "${eta}" == "EB" ]]
		then
		    echo "common_cut=(${eta_cut})" >> "${cut}"
		elif [[ "${eta}" == "EE" ]]
		then
		    echo "common_cut=(!${eta_cut})" >> "${cut}"
		elif [[ "${eta}" == "Full" ]]
		then
		    echo "common_cut=(1)" >> "${cut}"
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

		plot="tmp_plot_config.${inTextExt}"
		> "${plot}"
	    
		echo "plot_title=${title} (${eta})" >> "${plot}"
		echo "x_title=${title} (${eta})" >> "${plot}"
		echo "x_var=${x_var}" >> "${plot}"
		echo "x_bins=${x_bins}" >> "${plot}"

		###########################
         	## make plot config (2D) ##
		###########################

		plot2D="tmp_plot_config_2D.${inTextExt}"
		> "${plot2D}"
	    
		echo "plot_title=${time_title} vs. ${title} (${eta})" >> "${plot2D}"
		echo "x_title=${title} (${eta})" >> "${plot2D}"
		echo "x_var=${x_var}" >> "${plot2D}"
		echo "x_bins=${x_bins}" >> "${plot2D}"
		echo "y_title=${time_title} (${eta})" >> "${plot2D}"
		echo "y_var=${time_var}" >> "${plot2D}"
		echo "y_bins=${time_bins}" >> "${plot2D}"       

		echo "y_var_data=${data_corr}" >> "${plot2D}"
		echo "y_var_bkgd=${mc_corr}" >> "${plot2D}"
		echo "y_var_sign=${mc_corr}" >> "${plot2D}"

		## add corrections if plotting time
		if [[ "${var}" == "time" ]]
		then
		    echo "x_var_data=${data_corr}" >> "${plot}"
		    echo "x_var_bkgd=${mc_corr}" >> "${plot}"
		    echo "x_var_sign=${mc_corr}" >> "${plot}"
	
		    echo "x_var_data=${data_corr}" >> "${plot2D}"
		    echo "x_var_bkgd=${mc_corr}" >> "${plot2D}"
		    echo "x_var_sign=${mc_corr}" >> "${plot2D}"
		fi

		#########################
		## make timefit config ##
		#########################


		##############################
		## make misc timefit config ##
		##############################
		
		misc_fit="tmp_misc_fit.${inTextExt}"
		> "${misc_fit}"

		check_logx=$( CheckLogX ${var} )
		if [[ "${check_logx}" == "true" ]]
		then
		    echo "do_logx=1" >> "${misc_fit}"
		else
		    echo "do_logx=0" >> "${misc_fit}"
		fi

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
			    ##################
                 	    ## outfile name ##
                	    ##################
			    time="timefit"
			    outdir="${outdirbase}/${label}/${eta}/${var}"
			    outfile="${x_var}_${label}_${eta}_${era}"
			    outfile2D="${time_var}_vs_${outfile}"
		
			    ./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cut}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plot}" "${miscconfigdir}/${misc}.${inTextExt}" "${era}" "${outfile}" "${outdir}"
			    ./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cut}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plot2D}" "${miscconfigdir}/${misc}.${inTextExt}" "${era}" "${outfile2D}" "${outdir}"
			    ./scripts/runTimeFitter.sh "${outfile2D}_${era}.root" "${plot2D}" "${fit_misc}" "${timefitconfig}" "${era}" "${outfile}_${time}" "${outdir}"
			done ## read input
		    done ## loop over inputs
		done ## loop over eras

		## remove tmp files
		rm "${plot}" "${plot2D}" "${misc_fit}"

	    done ## loop over etas
	done ## loop over vars
    done ## read pho
done ## loop over phos
