#!/bin/bash

#######################
## Source This First ##
#######################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## i/o params
indir=${1:-"input"}
outname=${2-"AsymLim"}
outdir=${3:-"output"}

###########################################
## Ship things over to combine directory ##
###########################################

cp "${indir}/${base_datacardABCD}*.txt" "${combdir}"
cp "${indir}/${base_wsfileABCD}*.root" "${combdir}"

#####################
## Now work there! ##
#####################

pushd "${combdir}"
eval `scram runtime -sh`

#################################################
## Loop over signals and run asymptotic limits ##
#################################################

for lambda in 100 150 200 250 300 350 400 500 600
do
    for ctau in 0p001 0p1 10 200 400 600 800 1000 1200 10000 
    do
	name="GMSB_L${lambda}_CTau${ctau}"
	echo "Working on ${name}"

	combine -M AsymptoticLimits "${base_datacardABCD}_${name}.${inTextExt}" --run=expected --name "${name}"
    done
done

############
## rename ##
############

rename "higgsCombine" "${outname}" "*.root"
rename ".AsymptoticLimits.mH120" "" "*.root"

################
## Move back! ##
################

popd
eval `scram runtime -sh`

###########################################
## Ship things over to combine directory ##
###########################################

cp "${combdir}/${outname}*.root" "${outdir}"

###################
## Final message ##
###################

echo "Finished ExtractingResultsABCD"
