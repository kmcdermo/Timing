#!/bin/bash

## common input info
export skimdir="skims"

## config input info
export inTextExt="txt"
export tmplExt="tmpl"
export standardplotlist="standard_plots"
export reducedplotlist="reduced_analysis_plots"

## config dir info
export cutconfigdir="cut_config"
export fitconfigdir="fit_config"
export miscconfigdir="misc_config"
export plotconfigdir="plot_config"
export rescaleconfigdir="rescale_config"
export fragdir="plot_config/fragments"
export limitconfigdir="limit_config"
export systdir="syst_config"

## common output info
export outTextExt="log"
export topdir="/eos/user/k/kmcdermo/www"
export disphodir="dispho/plots"

## combine info
export combdir="/afs/cern.ch/work/k/kmcdermo/private/dispho/Analysis/combine/CMSSW_8_1_0/src/HiggsAnalysis/CombinedLimit/working"

## common derived output info
export limitdir="limits"

declare -a exts=("png" "pdf")
export exts

declare -a canvscales=("lin" "log")
export canvscales

## eras
declare -a eras=("Full")
export eras
export MainEra="Full"

## etas
declare -a etas=("EB" "EE")
export etas

## phos
export pho0="0 Leading"
export pho1="1 Subleading"
declare -a phos=("pho0")
export phos

## base time variable for input to resolution + corrections
export base_time_var="weightedtimeLT120"

## time corrections
export tof_corr="pho${base_time_var}TOF"
export shift_corr="pho${base_time_var}SHIFT"
export smear_corr="pho${base_time_var}SMEAR"

## time fit info
export fitinfo="Gaus1core 2 2"

## time adjust variables
export base_adjust_var="phoE"
export adjust_var="${base_adjust_var}_0"

## CR + SR info: label skim signal_skim additional_cuts (var wgt map)
export CR_GJets="cr_gjets_DEG v3/orig_2phosCR/gjets v3/orig_2phosCR/signals_gjets cuts_v1/lepveto"
export CR_QCD="cr_qcd_DEG v3/orig_2phosCR/qcd v3/orig_2phosCR/signals_qcd cuts_v1/lepveto"
export CR_DYLL="cr_dyll_DEG v3/orig_2phosCR/dyll v3/orig_2phosCR/signals_dyll always_true"
export SR="sr_SPH v3/orig_2phosCR/sr v3/orig_2phosCR/signals_sr cuts_v1/lepveto"

#declare -a inputs=(CR_GJets CR_QCD SR)
declare -a inputs=(SR)

## Zee 
#export Zee="Zee zee_TnP/skim zee_TnP/signals_skim always_true"
#export Zee="Zee zee_timestudy/skim zee_timestudy/signals_skim always_true"
#export Zee="Dixtal dixtal/skim dixtal/signals_skim always_true"
#declare -a inputs=(Zee)
#export inputs

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

    if [[ "${input}" == *"SR"* ]] 
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
