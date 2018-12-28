#!/bin/bash

## combine info
export combdir="/afs/cern.ch/work/k/kmcdermo/private/dispho/Analysis/combine/CMSSW_8_1_0/src/HiggsAnalysis/CombinedLimit/working"
export datacard="datacard.txt"

## config info
export inTextExt="txt"

## config dir info
export limitconfigdir="limit_config"

## common output info
export topdir="/eos/user/k/kmcdermo/www"
export disphodir="dispho/plots"

declare -a exts=("eps" "png" "pdf")
export exts

## era
export MainEra="Full"

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
