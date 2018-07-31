#!/bin/bash

## common input info
export skimdir="skims"

## config input info
export inTextExt="txt"
export tmplExt="tmpl"
export standardplotlist="standard_plots"

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

## CR + SR info: label skim signal_skim additional_cuts (var wgt map)
export GJets="cr_gjets_DEG orig/gjets orig/signals_gjets always_true"
export QCD="cr_qcd_DEG orig/qcd orig/signals_qcd cuts_v3/invertiso0_v0"
export Signal="sr_SPH orig/sr orig/signals_sr always_true"

export CR_GJets="${GJets} gjets_phopt_0_map"
export CR_QCD="${QCD} qcd_phopt_0_map"
export SR="${Signal} empty"
declare -a inputs=(CR_GJets CR_QCD SR)
export inputs

## function to make directory readable
function PrepOutDir ()
{
    fulldir=${1:-"${topdir}/${disphodir}/plots"}
    mkdir -p ${fulldir}

    pushd ${topdir}
    ./makereadable.sh ${fulldir}
    popd
}
export -f PrepOutDir

## function to decide which misc to use
function GetMisc ()
{
    input=${1}
    plot=${2}

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
