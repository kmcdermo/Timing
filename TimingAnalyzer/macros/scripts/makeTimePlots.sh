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
usetof=${3:-"false"}
useshift=${4:-"false"}
usesmear=${5:-"false"}
writefiles=${6:-"false"}
filedump=${7:-"${adjust_var}_infiles.${inTextExt}"}

## create filedump
if [[ "${writefiles}" == "true" ]]
then
    > "${filedump}"
fi

## vars
declare -a vars=("E" "${base_time_var}")

## logx vars
declare -a logx_vars=("pt" "E" "seedE")

## do full era vars for mu hists
declare -a mualleras_vars=()

## sigma fit vars
pt="p_{T} GeV 0 10 100 0 1 10"
E="E GeV 0 10 100 0 1 10"
seedE="seedE GeV 0 1 10 0 1 10"
declare -a sigmafit_vars=(pt E seedE)

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
	    ## only plot time vars for pho1
	    if [[ "${pho}" == "pho1" ]] && [[ "${var}" != *"time"* ]]
	    then
		continue
	    fi

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

	    ## add in photon index
	    if [[ "${var}" != "nvtx" ]] 
	    then
		title="${pho_label} ${title}"
		x_var="${x_var}_${index}"
	    fi

	    ## add units to title
	    if [[ "${unit}" != "" ]]
	    then
		title+=" ${unit}"
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
		elif [[ "${line}" == "unit="* ]]
		then
		    time_unit=$( ReadConfig "${line}" )
		elif [[ "${line}" == "bins="* ]]
		then
		    time_bins=$( ReadConfig "${line}" )
		fi
	    done < "${fragdir}/${base_time_var}.${inTextExt}" ## time or weightedtimeLT120

	    ## add units to title
	    time_title+=" ${time_unit}"
	    
	    #####################
	    ## set corrections ##
	    #####################

	    data_corr=""
	    mc_corr=""

	    if [[ "${usetof}" == "true" ]]
	    then
		tof_corr_index="+${tof_corr}_${index}"

		data_corr+="${tof_corr_index}"
		mc_corr+="${tof_corr_index}"
	    fi

	    if [[ "${useshift}" == "true" ]]
	    then
		shift_corr_index="+${shift_corr}_${index}"
		
		data_corr+="${shift_corr_index}"
		mc_corr+="${shift_corr_index}"
	    fi

	    if [[ "${usesmear}" == "true" ]]
	    then
		smear_corr_index="+${smear_corr}_${index}"

		mc_corr+="${smear_corr_index}"
	    fi

	    ## loop over eta regions
	    for eta in "${etas[@]}"
	    do
		## only do Full detector for time plot only
		if [[ "${eta}" == "Full" ]] && [[ "${var}" != *"time"* ]]
		then
		    continue
		fi

		#####################
		## make cut config ##
		#####################

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

		###########################
         	## make plot config (2D) ##
		###########################

		plot2D="tmp_pho_${index}_time_vs_${var}_${eta}.${inTextExt}"
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

		## add corrections if plotting time --> currently disabled time vs time plot
		if [[ "${var}" == *"time"* ]]
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

		misc=$( GetMisc ${input} ${plot} )

		#########################
		## make timefit config ##
		#########################
		
		timefit_config="tmp_timefit_config.${inTextExt}"
		> "${timefit_config}"

		echo "time_text=t" >> "${timefit_config}"
		echo ${fitinfo} | while read -r fittype rangelow rangeup
		do
		    echo "fit_type=${fittype}" >> "${timefit_config}"
		    echo "range_low=${rangelow}" >> "${timefit_config}"
		    echo "range_up=${rangeup}" >> "${timefit_config}"
		done

		check_sigmafit=$( CheckVar ${var} "${sigmafit_vars[@]}" )
		if [[ "${check_sigmafit}" == "true" ]]
		then
		    echo "do_sigma_fit=1" >> "${timefit_config}"
		    echo "use_sqrt2=0" >> "${timefit_config}"

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

		check_logx=$( CheckVar ${var} "${logx_vars[@]}" )
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
		    ## skip if not needed to do all eras
		    check_mualleras=$( CheckVar ${var} "${mualleras_vars[@]}" )
		    if [[ "${check_mualleras}" != "true" ]] && [[ "${era}" != "Full" ]]
		    then
			continue
		    fi

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

			    ## run 2D plotter, passing 2D plots to make fits for all vars except vs time
			    if [[ "${var}" != *"time"* ]]
			    then
                 	        ## extra outfile names
				outfile2D="time_${index}_vs_${outfile}"
				timefile="timefit"

				## run 2D plotter
				./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cut}" "${plot2D}" "${miscconfigdir}/${misc}.${inTextExt}" "${era}" ${savemetadata} "${outfile2D}" "${outdir}"

				## run fitter, getting 2D plots from before
				./scripts/runTimeFitter.sh "${outfile2D}.root" "${plot2D}" "${misc_fit}" "${timefit_config}" "${era}" ${savemetadata} "${outfile}_${timefile}" "${outdir}"
				
			        ## write out time files for correction computations
				if [[ "${writefiles}" == "true" ]] && [[ "${x_var}" == "${adjust_var}" ]] && [[ "${eta}" != "Full" ]]
				then
				    echo "${eta}_${era}=${outfile}_${timefile}.root" >> "${filedump}"
				fi
			    fi ## end check over vars to fit
			done ## read input
		    done ## loop over inputs
		done ## loop over eras

		## remove tmp files
		rm "${cut}" "${plot}" "${plot2D}" "${timefit_config}" "${misc_fit}"

	    done ## loop over etas
	done ## loop over vars
    done ## read pho
done ## loop over phos
