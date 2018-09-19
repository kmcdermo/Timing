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
usetof=${2:-"false"}
useshift=${3:-"false"}
usesmear=${4:-"false"}

## other info
diphodir="dipho"
fragdir="plot_config/fragments"

## eta regions
declare -a dietas=("EBEB")

## vars
declare -a vars_map=("seedE_eff seedE")

## logx vars
declare -a logx_vars=("pt_0" "pt_1" "pt_eff" "E_0" "E_1" "E_eff" "seedE_eff")

## do full era vars for mu hists
declare -a mualleras_vars=()

## sigma fit vars
pt_0="p_{T} GeV/c 0 10 100 0 1 10"
pt_1="p_{T} GeV/c 0 10 100 0 1 10"
pt_eff="p_{T}^{Eff} GeV/c 0 10 100 0 1 10"
E_0="E GeV 0 10 100 0 1 10"
E_1="E GeV 0 10 100 0 1 10"
E_eff="E_{eff} GeV 0 10 100 0 1 10"
seedE_eff="seedE_{eff} GeV 0 1 10 0 1 10"
declare -a sigmafit_vars=(pt_0 pt_1 pt_eff E_0 E_1 E_eff seedE_eff)

###############
## Run code! ##
###############

## loop over vars, dietas, eras, inputs
## write tmp configs as needed
## run 1D plots, 2D plots, and time fitter

for var_pair in "${vars_map[@]}"
do 
    echo ${var_pair} | while read -r var fragment
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
	    elif [[ "${line}" == "unit="* ]]
	    then
		unit=$( ReadConfig "${line}" )
	    elif [[ "${line}" == "ytitle="* ]]
	    then
		ytitle=$( ReadConfig "${line}" )
	    fi
	done < "${fragdir}/${fragment}.${inTextExt}"
    
        ## add in photon indices
	if   [[ "${var}" == *"_0" ]] 
	then
	    title="Leading ${title}"
	    x_var="${x_var}_0"
	elif [[ "${var}" == *"_1" ]]
	then
	    title="Subleading ${title}"
	    x_var="${x_var}_1"
	elif [[ "${var}" == *"_eff" ]]
	then
	    title="Effective ${title}"
	    x_var="((${x_var}_0*${x_var}_0)/sqrt(pow(${x_var}_0,2)+pow(${x_var}_1,2)))"
	elif [[ "${var}" == *"_delta" ]]
	then
	    title="#Delta(${title})"
	    x_var="${x_var}_0-${x_var}_1"
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
	    elif [[ "${line}" == "title="* ]]
	    then
		time_title=$( ReadConfig "${line}" )
	    elif [[ "${line}" == "unit="* ]]
	    then
		time_unit=$( ReadConfig "${line}" )
	    elif [[ "${line}" == "bins="* ]]
	    then
		time_bins=$( ReadConfig "${line}" )
	    fi
	done < "${fragdir}/time.${inTextExt}"
	
        ## add in photon indices
	time_var="${time_var}_0-${time_var}_1"

        ## add delta and units to title
	time_title="#Delta(${time_title}) ${time_unit}"
    
        #####################
        ## set corrections ##
        #####################

	## deltaT first, single T after
	time_data_corr=""
	time_mc_corr=""
	data_corr=""
	mc_corr=""

	## tof
	if [[ "${usetof}" == "true" ]]
	then
	    time_tof_corr_delta="+${tof_corr}_0-${tof_corr}_1"

	    time_data_corr+="${time_tof_corr_delta}"
	    time_mc_corr+="${time_tof_corr_delta}"

	    if [[ "${var}" == "time_0" ]]
	    then
		tof_corr_0="+${tof_corr}_0"

		data_corr+="${tof_corr_0}"
		mc_corr+="${tof_corr_0}"
	    elif [[ "${var}" == "time_1" ]]
	    then
		tof_corr_1="+${tof_corr}_1"

		data_corr+="${tof_corr_1}"
		mc_corr+="${tof_corr_1}"
	    elif [[ "${var}" == "time_delta" ]]
	    then
		data_corr+="${time_tof_corr_delta}"
		mc_corr+="+${time_tof_corr_delta}"
	    fi
	fi

	## shift
	if [[ "${useshift}" == "true" ]]
	then
	    time_shift_corr_delta="+${shift_corr}_0-${shift_corr}_1"

	    time_data_corr+="${time_shift_corr_delta}"
	    time_mc_corr+="${time_shift_corr_delta}"

	    if [[ "${var}" == "time_0" ]]
	    then
		shift_corr_0="+${shift_corr}_0"

		data_corr+="${shift_corr_0}"
		mc_corr+="${shift_corr_0}"
	    elif [[ "${var}" == "time_1" ]]
	    then
		shift_corr_1="+${shift_corr}_1"

		data_corr+="${shift_corr_1}"
		mc_corr+="${shift_corr_1}"
	    elif [[ "${var}" == "time_delta" ]]
	    then
		data_corr+="${time_shift_corr_delta}"
		mc_corr+="+${time_shift_corr_delta}"
	    fi
	fi

	## smear
	if [[ "${usesmear}" == "true" ]]
	then
	    time_smear_corr_delta="+${smear_corr}_0-${smear_corr}_1"

	    time_mc_corr+="${time_smear_corr_delta}"

	    if [[ "${var}" == "time_0" ]]
	    then
		smear_corr_0="+${smear_corr}_0"

		mc_corr+="${smear_corr_0}"
	    elif [[ "${var}" == "time_1" ]]
	    then
		smear_corr_1="+${smear_corr}_1"

		mc_corr+="${smear_corr_1}"
	    elif [[ "${var}" == "time_delta" ]]
	    then
		mc_corr+="+${time_smear_corr_delta}"
	    fi
	fi
    
        ## loop over dieta regions
	for eta in "${dietas[@]}"
	do
	    ## only do Full detector for time plot only
	    if [[ "${eta}" == "Full" ]] && [[ "${var}" != "time_delta" ]]
	    then
		continue
	    fi
	    
	    #########################
	    ## make eta cut config ##
	    #########################

	    cut="tmp_cut_config.txt"
	    > "${cut}"

	    ## write common cut
	    common_cut="(phoseedE_0>10&&phoseedE_0<120)&&(phoseedE_1>10&&phoseedE_1<120)"
	    eta_cut="phoisEB"
	    if [[ "${eta}" == "EBEB" ]]
	    then
		echo "common_cut=((${eta_cut}_0&&${eta_cut}_1)&&(${common_cut}))" >> "${cut}"
	    elif [[ "${eta}" == "EBEE" ]]
	    then
		echo "common_cut=(((${eta_cut}_0&&!${eta_cut}_1)||(!${eta_cut}_0&&${eta_cut}_1))&&(${common_cut}))" >> "${cut}"
	    elif [[ "${eta}" == "EEEE" ]]
	    then
		echo "common_cut=((!${eta_cut}_0&&!${eta_cut}_1)&&(${common_cut}))" >> "${cut}"
	    elif [[ "${eta}" == "Full" ]]
	    then
		echo "common_cut=(${common_cut})" >> "${cut}"
	    else
		echo "How did this happen?? Did not choose a correct option for dieta: ${eta} ... Exiting..."
		exit
	    fi
	    
	    ## write the remainder of cuts
	    echo "data_cut=" >> "${cut}"
	    echo "bkgd_cut=" >> "${cut}"
	    echo "sign_cut=" >> "${cut}"
	    
	    ###########################
            ## make plot config (1D) ##
	    ###########################
	    
	    plot="tmp_${var}_${eta}.${inTextExt}"
	    > "${plot}"
	    
	    echo "plot_title=${title} (${eta})" >> "${plot}"
	    echo "x_title=${title} (${eta})" >> "${plot}"
	    echo "x_var=${x_var}" >> "${plot}"
	    echo "x_bins=${x_bins}" >> "${plot}"
	    echo "y_title=${ytitle}" >> "${plot}"

	    ###########################
            ## make plot config (2D) ##
	    ###########################

	    plot2D="tmp_deltaT_vs_${var}_${eta}.${inTextExt}"
	    > "${plot2D}"

	    echo "plot_title=${time_title} vs. ${title} (${eta})" >> "${plot2D}"
	    echo "x_title=${title} (${eta})" >> "${plot2D}"
	    echo "x_var=${x_var}" >> "${plot2D}"
	    echo "x_bins=${x_bins}" >> "${plot2D}"
	    echo "y_title=${time_title} (${eta})" >> "${plot2D}"
	    echo "y_var=${time_var}" >> "${plot2D}"
	    echo "y_bins=${time_bins}" >> "${plot2D}"       
	    
	    echo "y_var_data=${time_data_corr}" >> "${plot2D}"
	    echo "y_var_bkgd=${time_mc_corr}" >> "${plot2D}"
	    echo "y_var_sign=${time_mc_corr}" >> "${plot2D}"
	    
	    ## add corrections if plotting time --> currently disabled time_delta vs time_delta plot
	    if [[ "${var}" == "time"* ]]
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
	    
	    echo "time_text=#Delta(T)" >> "${timefit_config}"
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
		echo "use_sqrt2=1" >> "${timefit_config}"
		    
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
		do echo ${!input} | while read -r label infile insigfile sel varwgtmap
		    do
                 	## outfile names
			outdir="${outdirbase}/${label}/${diphodir}/${eta}/${var}"
			outfile="${var}_${label}_${eta}_${era}"
			
			## run 1D plotter
			./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cut}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plot}" "${miscconfigdir}/${misc}.${inTextExt}" "${era}" "${outfile}" "${outdir}"

			## run 2D plotter, passing 2D plots to make fits for all vars except vs time
			if [[ "${var}" != "time_delta" ]]
			then
                 	    ## extra outfile names
			    outfile2D="deltaT_vs_${outfile}"
			    timefile="timefit"

			    ## run 2D plotter
			    ./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cut}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plot2D}" "${miscconfigdir}/${misc}.${inTextExt}" "${era}" "${outfile2D}" "${outdir}"

			    ## run fitter, getting 2D plots from before
			    ./scripts/runTimeFitter.sh "${outfile2D}.root" "${plot2D}" "${misc_fit}" "${timefit_config}" "${era}" "${outfile}_${timefile}" "${outdir}"
				
			fi ## end check over vars to fit
		    done ## read input
		done ## loop over inputs
	    done ## loop over eras

	    ## remove tmp files
	    rm "${cut}" "${plot}" "${plot2D}" "${timefit_config}" "${misc_fit}"

	done ## loop over dietas
    done ## read var_pair
done ## loop over vars_map
