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
export systconfigdir="syst_config"

## common output info
export outTextExt="log"
export topdir="/eos/user/k/kmcdermo/www"
export disphodir="dispho/plots"

## combine info
export combdir="/afs/cern.ch/work/k/kmcdermo/private/dispho/Analysis/combine/CMSSW_8_1_0/src/HiggsAnalysis/CombinedLimit/working"

## category info
export ex1pho="exclusive_1pho"
export in2pho="inclusive_2pho"
export categorydir="combined_categories"

## common info for optimized ABCD scan
export plotfiletext="met_vs_time" # 2D plot base name
export scan_log="abcd_scan_for_r.${outTextExt}" # Limit Scan Log File

export inlimitdir="input" # directory for datacards
export outlimitdir="output" # directory for output root files from combine, limit plots

export datacardname="datacardABCD" # datacard base name as input to combine
export combinelogname="combine" # log file base name from running combine
export outcombname="AsymLim" # root file base name from running combine

export incombdir="combine_input" # for www: Datacards fed to combine, as well binning configuration info
export outlimitplotdir="limits" # for www: ROOT and log files from running combine, and limit plots
export outplot2Ddir="plots2D" # for www: Input 2D plots file for building datacards

export limit="limits2D" # limit config used for 2D plot
export outlimit1D="limit1D" # output base name for 1D limits
export outlimit2D="limit2D" # output base name for 2D limits

## common info for systematic studies
export nom2Ddir="nominal2D"
export unc2Ddir="systunc2D"

## plotting config
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

## CR + SR info: label skim signal_skim additional_cuts
export CR_GJets="gjets v4/final/gjets v4/final/signals_gjets always_true"
export CR_QCD="qcd v4/final/qcd v4/final/signals_qcd always_true"
export CR_EWK="ewk v4/final/ewk_trk v4/final/signals_ewk_trk always_true"
#export CR_EWK="ewk v4/final/ewk v4/final/signals_ewk always_true"

## Unc tests for signals
export SR="sr_SPH v4/categories/v1/inclusive_2pho v4/categories/v1/signals_inclusive_2pho always_true"
export PHO_SCALE_UP="phoscaleup NO_DATA v4/unc_tests/signals_PhoScaleUp always_true"
export PHO_SCALE_DOWN="phoscaledown NO_DATA v4/unc_tests/signals_PhoScaleDown always_true"
export PHO_SMEAR_UP="phosmearup NO_DATA v4/unc_tests/signals_PhoSmearUp always_true"
export PHO_SMEAR_DOWN="phosmeardown NO_DATA v4/unc_tests/signals_PhoSmearDown always_true"
export JET_SCALE_UP="jetscaleup NO_DATA v4/unc_tests/signals_JetScaleUp always_true"
export JET_SCALE_DOWN="jetscaledown NO_DATA v4/unc_tests/signals_JetScaleDown always_true"
export JET_RESOL_UP="jetresolup NO_DATA v4/unc_tests/signals_JetResolUp always_true"
export JET_RESOL_DOWN="jetresoldown NO_DATA v4/unc_tests/signals_JetResolDown always_true"

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
