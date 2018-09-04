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
usecorr=${4:-"false"}

## other info
fragdir="plot_config/fragments"}
timefitdir="timefit_config"}
infile="zee"
infile="zee_signals"
varwgtmap="empty"
misc="misc"

## eras
declare -a eras=(2017B 2017C 2017D 2017E 2017F)

## etas
EB="0 1.4442"
EE="1.566 2.5"
Full="0 2.5"
declare -a etas=(EB EE Full)

## vars
declare -a vars=(E pt eta time nvtx)

## phos
pho0="0 Leading"
pho1="1 Subleading"
declare -a phos=(pho0 pho1)

## function
function ReadConfig ()
{
    local line=${1}
    echo "${line}" | cut -d "=" -f 2
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
		elif [[ "${line}" == "label="* ]]
		then
		    label=$( ReadConfig "${line}" )
		elif [[ "${line}" == "bins="* ]]
		then
		    x_bins=$( ReadConfig "${line}" )
		fi
	    done < "${fragdir}/${var}.${inTextExt}"

	    ## add in photon index
	    if [[ "${var}" != "nvtx" ]] 
	    then
		label="${pho_label} ${label}"
		x_var="${x_var}_${index}"
	    fi

	    ## read in time plot info (2D)
	    while IFS='' read -r line || [[ -n "${line}" ]]
	    do
		if   [[ "${line}" == "var="* ]]
		then
		    time_var=$( ReadConfig "${line}" )
		    time_var="${time_var}_${index}"
		elif [[ "${line}" == "label="* ]]
		then
		    time_label=$( ReadConfig "${line}" )
		    time_label="${pho_label} ${time_label}"
		elif [[ "${line}" == "bins="* ]]
		then
		    time_bins=$( ReadConfig "${line}" )
		fi
	    done < "${fragdir}/time.${inTextExt}"
	    
	    ## set corrections
	    std_corr="+phoseedTOF_${pho_label}"
	    data_corr="${std_corr}"
	    mc_corr="${std_corr}"

	    if [[ "${usecorr}" == "true" ]]
	    then
		data_corr="+phoseedtimeSHIFT_${pho_label}" 
		mc_corr="+phoseedtimeSHIFT_${pho_label}+phoseedtimeSMEAR_${pho_label}"
	    fi

	    ## loop over eta regions
	    for eta in "${etas[@]}"
	    do
		## make eta cut config
		cut="tmp_cut_config.txt"
		> "${cut}"

		echo ${!eta} | while read -r eta_low eta_high
		do
		    echo "common_cut=(abs(phoeta_${index})>=${eta_low}&&abs(phoeta_${index})<=${eta_high})" >> "${cut}"
		    echo "data_cut=" >> "${cut}"
		    echo "bkgd_cut=" >> "${cut}"
		    echo "sign_cut=" >> "${cut}"
		done ## read eta

		###########################
         	## make plot config (1D) ##
		###########################

		plot="tmp_plot_config.txt"
		> "${plot}"
	    
		echo "plot_title=${label} (${eta})" >> "${plot}"
		echo "x_title=${label} (${eta})" >> "${plot}"
		echo "x_var=${x_var}" >> "${plot}"
		echo "x_bins=${x_bins}" >> "${plot}"

		###########################
         	## make plot config (2D) ##
		###########################

		plot2D="tmp_plot_config_2D.txt"
		> "${plot2D}"
	    
		echo "plot_title=${time_label} vs. ${label} (${eta})" >> "${plot2D}"
		echo "x_title=${label} (${eta})" >> "${plot2D}"
		echo "x_var=${x_var}" >> "${plot2D}"
		echo "x_bins=${x_bins}" >> "${plot2D}"
		echo "y_title=${time_label} (${eta})" >> "${plot2D}"
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
		
		##################
		## outfile name ##
		##################
		outdir="${outdirbase}/${eta}/${var}"
		outfile="${x_var}_${eta}"
		outfile2D="${time_var}_vs_${outfile}"
		extra="full_lumi"
		time="timefit"

		####################
		## loop over eras ##
		####################
		for era in "${eras[@]}"
		do
		    ./scripts/runEraTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cut}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plot}" "${miscconfigdir}/${misc}.${inTextExt}" "${outfile}_${era}" "${era}" "${outdir}"
		    
		    ./scripts/runEraTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cut}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plot2D}" "${miscconfigdir}/${misc}.${inTextExt}" "${outfile2D}_${era}" "${era}" "${outdir}"

		    ./scripts/runTimeFitter.sh "${outfile2D}_${era}.root" "${plot2D}" "${timefitdir}/${timefitconfig}.${inTextExt}" "${era}" "${outfile}_${era}_${time}" "${outdir}"
		done ## end loop over eras

		## make full lumi plots
		./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cut}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plot}" "${miscconfigdir}/${misc}.${inTextExt}" "${outfile}_${extra}" "${outdir}"
		./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cut}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plot2D}" "${miscconfigdir}/${misc}.${inTextExt}" "${outfile2D}_${extra}" "${outdir}"
		./scripts/runTimeFitter.sh "${outfile2D}.root" "${plot2D}" "${timefitdir}/${timefitconfig}.${inTextExt}" "full" "${outfile}_${extra}_${time}" "${outdir}"

		## remove tmp files
		rm "${plot}" "${plot2D}"
	    done ## loop over etas
	done ## loop over vars
    done ## read pho
done ## loop over phos
