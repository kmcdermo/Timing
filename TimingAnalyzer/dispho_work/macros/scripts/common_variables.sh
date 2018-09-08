#!/bin/bash

## common input info
export skimdir="skims"

## config input info
export inTextExt="txt"
export tmplExt="tmpl"
export standardplotlist="standard_plots"
export reducedplotlist="reduced_analysis_plots"

## config dir info
export crtosrconfigdir="crtosr_config"
export cutconfigdir="cut_config"
export fitconfigdir="fit_config"
export miscconfigdir="misc_config"
export plotconfigdir="plot_config"
export rescaleconfigdir="rescale_config"
export srplotconfigdir="srplot_config"
export varwgtconfigdir="varwgt_config"
export fragdir="plot_config/fragments"

## common output info
export outTextExt="log"
export topdir="/eos/user/k/kmcdermo/www"
export disphodir="dispho/plots"

## common derived output info
export limitdir="limits"

declare -a exts=("eps" "png" "pdf")
export exts

declare -a canvscales=("lin" "log")
export canvscales

## eras
declare -a eras=("2017B" "2017C" "2017D" "2017E" "2017F" "Full")
export eras
export MainEra="Full"

## etas
declare -a etas=("EB" "EE" "Full")
export etas

## phos
export pho0="0 Leading"
export pho1="1 Subleading"
declare -a phos=("pho0" "pho1")
export phos

## time corrections
export tof_corr="phoseedTOF"
export shift_corr="phoseedtimeSHIFT"
export smear_corr="phoseedtimeSMEAR"

## weight info
export wgtvar="phopt_0"

## fit info
export fitinfo="Gaus1core 2 2"

## time adjust var
export timeadjvar="phopt_0"

## CR + SR info: label skim signal_skim additional_cuts (var wgt map)
export GJets="cr_gjets_DEG v4/orig/gjets v4/orig/signals_gjets always_true"
export QCD="cr_qcd_DEG v4/orig/qcd v4/orig/signals_qcd always_true"
export Signal="sr_SPH v4/orig/sr v4/orig/signals_sr always_true"

export CR_GJets="${GJets} gjets_${wgtvar}_map"
export CR_QCD="${QCD} qcd_${wgtvar}_map"
export SR="${Signal} empty"
#declare -a inputs=(CR_GJets CR_QCD SR)

## Zee 
export Zee="Zee_DEG v4_metcorr_Zee/zee v4_metcorr_Zee/signals_zee cuts_v4/zee"
export ZEE="${Zee} empty"
declare -a inputs=(ZEE)
export inputs

################################
## Generic function overrides ##
################################

## suppress pushd, popd output
function pushd () 
{
    command pushd "$@" > /dev/null
}
export -f pushd

function popd () 
{
    command popd "$@" > /dev/null
}
export -f popd

####################
## Plot functions ##
####################

## function to make directory readable
function PrepOutDir ()
{
    local fulldir=${1:-"${topdir}/${disphodir}"}
    mkdir -p ${fulldir}

    pushd ${topdir}
    ./copyphp.sh ${fulldir}
    popd
}
export -f PrepOutDir

## function to decide which misc to use
function GetMisc ()
{
    local input=${1}
    local plot=${2}

    local misc="misc"

    if [[ "${input}" == "SR" ]] 
    then
	if [[ "${plot}" == *"met"* ]] || [[ "${plot}" == *"time"* ]] 
	then
	    misc="misc_blind"
	fi
    fi

    if [[ "${input}" == "ZEE" ]] 
    then
	if [[ "${plot}" != *"SMEAR"* ]]
	then
	    misc="misc_zee"
	else
	    misc="misc_zee_smear"
	fi
    fi

    echo "${misc}"
}
export -f GetMisc

########################
## Time Fit Functions ##
########################

## function to read config
function ReadConfig ()
{
    local line=${1}
    echo "${line}" | cut -d "=" -f 2
}
export -f ReadConfig

## function to say if logx
function CheckVar ()
{
    local var=${1}
    shift
    local arr_vars=("$@")
    local result="false"

    for arr_var in "${arr_vars[@]}"
    do
	if [[ "${var}" == "${arr_var}" ]]
	then
	    result="true"
	    break
	fi
    done
    
    echo "${result}"
}
export -f CheckVar
