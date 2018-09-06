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
useshift=${2:-"false"}
usesmear=${3:-"false"}
fittype=${4:-"Gaus1core"}
rangelow=${5:-"3"}
rangeup=${6:-"3"}
writefiles=${7:-"false"}
filedump=${8:-"${timeadjvar}_infiles.${inTextExt}"}

## create filedump
if [[ "${writefiles}" == "true" ]]
then
    > "${filedump}"
fi

## other info
fragdir="plot_config/fragments"

## etas
declare -a etas=("EB" "EE") # "Full"

## vars
declare -a vars=("pt" "eta" "nvtx") # "time" "E"

## logx vars
declare -a logx_vars=("pt") # "E"

## sigma fit vars
pt="p_{T} GeV/c 0 5 100 0 0.5 10"
declare -a sigmafit_vars=(pt)

## phos
pho0="0 Leading"
pho1="1 Subleading"
declare -a phos=("pho0") # "pho1"

###############
## Functions ##
###############

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

## function to check if sigma fit
function CheckSigmaFitVar ()
{
    local var=${1}
    local result="false"

    for sigmafit_var in "${sigmafit_vars[@]}"
    do
	if [[ "${var}" == "${sigmafit_var}" ]]
	then
	    result="true"
	    break
	fi
    done
    
    echo "${result}"
}

###############
## Run code! ##
###############

## loop over photos, vars, etas, eras, inputs
## write tmp configs as needed
## run 1D plots, 2D plots, and time fitter

for pho in "${phos[@]}"
do echo ${!pho} | while read -r index pho_label
    do
	for var in "${vars[@]}"
	do 
	    ##########################
	    ## Set plot config (1D) ##
	    ##########################

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
		elif [[ "${line}" == "ytitle="* ]]
		then
		    ytitle=$( ReadConfig "${line}" )
		fi
	    done < "${fragdir}/${var}.${inTextExt}"

	    ## add in photon index
	    if [[ "${var}" != "nvtx" ]] 
	    then
		title="${pho_label} ${title}"
		x_var="${x_var}_${index}"
	    fi

	    ##########################
	    ## Set plot config (2D) ##
	    ##########################

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
	    
	    #####################
	    ## set corrections ##
	    #####################

	    tof_corr="+phoseedTOF_${index}"
	    shift_corr="+phoseedtimeSHIFT_${index}"
	    smear_corr="+phoseedtimeSMEAR_${index}"

	    data_corr="${tof_corr}"
	    mc_corr="${tof_corr}"

	    if [[ "${useshift}" == "true" ]]
	    then
		data_corr+="${shift_corr}"
		mc_corr+="${shift_corr}"
	    fi

	    if [[ "${usesmear}" == "true" ]]
	    then
		data_corr+="${smear_corr}"
		mc_corr+="${smear_corr}"
	    fi

	    ## loop over eta regions
	    for eta in "${etas[@]}"
	    do
		#########################
		## make eta cut config ##
		#########################

		cut="tmp_cut_config.txt"
		> "${cut}"

		## write common cut
		common_cut="hltDiEle27WPT&&!phoisOOT_0&&!phoisOOT_1&&phohasPixSeed_0&&phohasPixSeed_1"
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

		plot="tmp_plot_config.${inTextExt}"
		> "${plot}"
	    
		echo "plot_title=${title} (${eta})" >> "${plot}"
		echo "x_title=${title} (${eta})" >> "${plot}"
		echo "x_var=${x_var}" >> "${plot}"
		echo "x_bins=${x_bins}" >> "${plot}"
		echo "y_title=${ytitle}" >> "${plot}"

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

		######################################
		## determine which misc file to use ##
		######################################

		misc=$(GetMisc ${input} ${plot})

		#########################
		## make timefit config ##
		#########################
		
		timefit_config="tmp_timefit_config.${inTextExt}"
		> "${timefit_config}"

		echo "fit_type=${fittype}" >> "${timefit_config}"
		echo "range_low=${rangelow}" >> "${timefit_config}"
		echo "range_up=${rangeup}" >> "${timefit_config}"
		echo "time_text=t" >> "${timefit_config}"

		check_sigmafit=$( CheckSigmaFitVar ${var} )
		if [[ "${check_sigmafit}" == "true" ]]
		then
		    echo "do_sigma_fit=1" >> "${timefit_config}"
		    echo ${!var} | while read -r var_text var_unit N_low N_val N_up C_low C_val C_up
		    do
			echo "sigma_var_text=${var_text}" >> "${timefit_config}"
			echo "sigma_var_unit=${var_unit}" >> "${timefit_config}"
			echo "sigma_init_N_params=${N_low} ${N_val} ${N_up}" >> "${timefit_config}"
			echo "sigma_init_C_params=${C_low} ${C_val} ${C_up}" >> "${timefit_config}"
		    done
		else
		    echo "do_sigma_fit=0" >> "${timefit_config}"
		fi

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
			    timefile="timefit"
			    outdir="${outdirbase}/${label}/${eta}/${var}"
			    outfile="${x_var}_${label}_${eta}_${era}"
			    outfile2D="${time_var}_vs_${outfile}"
			    
			    ## run 1D, 2D, and time plots
			    ./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cut}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plot}" "${miscconfigdir}/${misc}.${inTextExt}" "${era}" "${outfile}" "${outdir}"
			    ./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cut}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plot2D}" "${miscconfigdir}/${misc}.${inTextExt}" "${era}" "${outfile2D}" "${outdir}"
			    ./scripts/runTimeFitter.sh "${outfile2D}.root" "${plot2D}" "${misc_fit}" "${timefit_config}" "${era}" "${outfile}_${timefile}" "${outdir}"

			    ## write out time files
			    if [[ "${writefiles}" == "true" ]] && [[ "${x_var}" == "${timeadjvar}" ]] && [[ "${eta}" != "Full" ]]
			    then
				echo "${eta}_${era}=${outfile}_${timefile}.root" >> "${filedump}"
			    fi
			    
			done ## read input
		    done ## loop over inputs
		done ## loop over eras

		## remove tmp files
		rm "${cut}" "${plot}" "${plot2D}" "${timefit_config}" "${misc_fit}"

	    done ## loop over etas
	done ## loop over vars
    done ## read pho
done ## loop over phos
