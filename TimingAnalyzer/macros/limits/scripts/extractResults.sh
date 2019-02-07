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
infile=${2:-"ws_final.root"}
outname=${3:-"AsymLim"}
outdir=${4:-"output"}
dir=${5:-"madv2_v3/full_chain/results/combine_input"}

## data card template
indatacard="${base_datacard}.${tmplExt}"

###########################################
## Ship things over to combine directory ##
###########################################

cp "${carddir}/${indatacard}" "${combdir}"
cp "${indir}/${infile}" "${combdir}"

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

	## make a new datacard for each signal, replacing PDF names
	tmpdatacard="${base_datacard}_${name}.${inTextExt}"
	cp "${indatacard}" "${tmpdatacard}"
	sed -i "s/SIGNAL_PDF/GMSB_L${lambda}_CTau${ctau}_PDF/g" "${tmpdatacard}"

	## also update to correct ws file!
	sed -i "s/INPUT_FILE/${infile}/g" "${tmpdatacard}"

	combine -M AsymptoticLimits "${tmpdatacard}" --run=blind --name "${name}"
    done
done

############
## rename ##
############

rename "higgsCombine" "${outname}" *.root
rename ".AsymptoticLimits.mH120" "" *.root

################
## Move back! ##
################

popd
eval `scram runtime -sh`

##########################################
## Ship things over to limits directory ##
##########################################

cp "${combdir}/${outname}"*.root "${outdir}"

######################################
## Copy Combine Inputs to Plot dirs ##
######################################

fulldir="${topdir}/${disphodir}/${dir}"
PrepOutDir "${fulldir}"

cp "${indir}/${infile}" "${fulldir}"
cp "${combdir}/${base_datacard}"*".${inTextExt}" "${fulldir}"

###################
## Final message ##
###################

echo "Finished ExtractingResults"
