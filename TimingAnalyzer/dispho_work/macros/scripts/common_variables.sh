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

## common output info
export outTextExt="log"
export topdir="/afs/cern.ch/user/k/kmcdermo/www"
export disphodir="dispho"

declare -a exts=("eps" "png" "pdf")
export exts

declare -a canvscales=("lin" "log")
export canvscales

## weight info
export wgtvar="phopt_0"

## CR + SR info: label skim signal_skim additional_cuts (var wgt map)
export GJets="cr_gjets_DEG v4/orig/gjets v4/orig/signals_gjets always_true"
export QCD="cr_qcd_DEG v4/orig/qcd v4/orig/signals_qcd always_true"
export Signal="sr_SPH v4/orig/sr v4/orig/signals_sr always_true"

export CR_GJets="${GJets} gjets_${wgtvar}_map"
export CR_QCD="${QCD} qcd_${wgtvar}_map"
export SR="${Signal} empty"
declare -a inputs=(CR_GJets CR_QCD SR)
export inputs

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

## function to make directory readable
function PrepOutDir ()
{
    local fulldir=${1:-"${topdir}/${disphodir}/plots"}
    mkdir -p ${fulldir}

    pushd ${topdir}
    ./makereadable.sh ${fulldir}
    popd
}
export -f PrepOutDir

## function to decide which misc to use
function GetMisc ()
{
    local input=${1}
    local plot=${2}

    local misc="misc"

    if [[ ${input} == "SR" ]] 
    then
	if [[ ${plot} == *"met"* ]] || [[ ${plot} == *"phoseedtime_0"* ]] 
	then
	    misc="misc_blind"
	fi
    fi
    
    echo "${misc}"
}
export -f GetMisc
