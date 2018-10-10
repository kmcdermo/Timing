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
triggertower=${5:-"Inclusive"}
effseedE=${6:-0} # (15 for Zee, 30 for dixtal)

## other info
var="runno"
diphodir="dipho"
fragdir="plot_config/fragments"

## eta regions
declare -a dietas=("EBEB")

###############
## Run code! ##
###############

## loop over dietas, inputs
## write tmp configs as needed
## run 2D plots and time fitter

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
    fi
done < "${fragdir}/${var}.${inTextExt}"

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
done < "${fragdir}/${base_time_var}.${inTextExt}"

## add in photon indices
time_var="${time_var}_0-${time_var}_1"

## add delta and units to title
time_title="#Delta(${time_title}) ${time_unit}"

#####################
## Set corrections ##
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
fi

## shift
if [[ "${useshift}" == "true" ]]
then
    time_shift_corr_delta="+${shift_corr}_0-${shift_corr}_1"
    time_data_corr+="${time_shift_corr_delta}"
    time_mc_corr+="${time_shift_corr_delta}"
fi

## smear
if [[ "${usesmear}" == "true" ]]
then
    time_smear_corr_delta="+${smear_corr}_0-${smear_corr}_1"
    time_mc_corr+="${time_smear_corr_delta}"
fi

## loop over dieta regions
for eta in "${dietas[@]}"
do    
    #####################
    ## Make cut config ##
    #####################

    cut="tmp_cut_config.txt"
    > "${cut}"

    ## eta cuts
    eta_cut_base="phoisEB"
    if [[ "${eta}" == "Full" ]]
    then
	eta_cut="(1)"
    elif [[ "${eta}" == "EBEB" ]]
    then
	eta_cut="(${eta_cut_base}_0&&${eta_cut_base}_1)"
    elif [[ "${eta}" == "EBEE" ]]
    then
	eta_cut="((${eta_cut_base}_0&&!${eta_cut_base}_1)||(!${eta_cut_base}_0&&${eta_cut_base}_1))"
    elif [[ "${eta}" == "EEEE" ]]
    then
	eta_cut="(!${eta_cut_base}_0&&!${eta_cut_base}_1)"
    else
	echo "How did this happen?? Did not choose a correct option for dieta: ${eta} ... Exiting..."
	exit
    fi
    
    ## trigger tower cuts
    if [[ "${triggertower}" == "Inclusive" ]]
    then
	TT_cut="(1)"
    elif [[ "${triggertower}" == "Same" ]]
    then
	TT_cut="(phoseedTT_0==phoseedTT_1)"
    elif [[ "${triggertower}" == "Different" ]]
    then
	TT_cut="(phoseedTT_0!=phoseedTT_1)"
    else
	echo "Yikes, triggertower cannot be: ${triggertower} ... Exiting..."
	exit
    fi

    ## effseedE cut
    effseedE_cut="(((phoseedE_0*phoseedE_1)/(sqrt(pow(phoseedE_0,2)+pow(phoseedE_1,2))))>${effseedE})"

    ## write the remainder of cuts
    echo "common_cut=${eta_cut}&&${TT_cut}&&${effseedE_cut}" >> "${cut}"
    echo "data_cut=" >> "${cut}"
    echo "bkgd_cut=" >> "${cut}"
    echo "sign_cut=" >> "${cut}"
    
    ###########################
    ## Make plot config (2D) ##
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
    
    ######################################
    ## Determine which misc file to use ##
    ######################################

    misc=$( GetMisc ${input} ${plot2D} )
    
    #########################
    ## Make timefit config ##
    #########################
    
    timefit_config="tmp_timefit_config.${inTextExt}"
    > "${timefit_config}"
    
    echo "time_text=#DeltaT" >> "${timefit_config}"
    echo ${fitinfo} | while read -r fittype rangelow rangeup
    do
	echo "fit_type=${fittype}" >> "${timefit_config}"
	echo "range_low=${rangelow}" >> "${timefit_config}"
	echo "range_up=${rangeup}" >> "${timefit_config}"
    done
    
    ################################
    ## Loop over inputs: Zee only ##
    ################################
    for input in "${inputs[@]}"
    do echo ${!input} | while read -r label infile insigfile sel varwgtmap
	do
            ## outfile names
	    outdir="${outdirbase}/${label}/${diphodir}/${eta}/${var}"
	    outfile="${var}_${label}_${eta}"
	    
            ## extra outfile names
	    outfile2D="deltaT_vs_${outfile}"
	    timefile="timefit"

	    ## run 2D plotter
	    ./scripts/runTreePlotter2D.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cut}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plot2D}" "${miscconfigdir}/${misc}.${inTextExt}" "${MainEra}" "${outfile2D}" "${outdir}"

	    ## run fitter, getting 2D plots from before
	    ./scripts/runTimeVsRunFitter.sh "${outfile2D}.root" "${plot2D}" "${timefit_config}" "${outfile}_${timefile}" "${outdir}"
	done ## read input
    done ## loop over inputs

    ## remove tmp files
    rm "${cut}" "${plot2D}" "${timefit_config}"
    
done ## loop over dietas
