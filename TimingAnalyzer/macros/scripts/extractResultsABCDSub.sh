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
datacardname=${2:-"datacardABCD"}
combine_extra=${3:-""}
logname=${4:-"combine"}

#################################################
## Loop over signals and run asymptotic limits ##
#################################################

for ctau in 0p001 0p1 10 200 400 600 800 1000 1200 10000
do
    sample="GMSB_L${lambda}_CTau${ctau}"
    echo "Running combine for: ${sample}"
    
    log="${logname}_${sample}.${outTextExt}"
    
    combine -M AsymptoticLimits "${datacardname}_${sample}.${inTextExt}" "${combine_extra}" --name "${sample}" >& "${log}"
done
