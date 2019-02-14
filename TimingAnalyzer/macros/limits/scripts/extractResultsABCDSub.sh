#!/bin/bash

#######################
## Source This First ##
#######################

source common_variables.sh

###################
## Configuration ##
###################

## i/o params
lambda=${1:-"100"}
combine_extra=${2:-""}
logname=${3:-"combine"}

#################################################
## Loop over signals and run asymptotic limits ##
#################################################

for ctau in 0p001 0p1 10 200 400 600 800 1000 1200
do
    sample="GMSB_L${lambda}_CTau${ctau}"
    echo "Working on ${sample}"
    
    log="${logname}_${sample}.${outTextExt}"
    
    combine -M AsymptoticLimits "${base_datacardABCD}_${sample}.${inTextExt}" "${combine_extra}" --name "${sample}" >& "${log}"
done
