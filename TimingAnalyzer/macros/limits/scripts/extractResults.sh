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
outname=${3-"AsymLim"}
outdir=${4:-"output"}

## other global vars
carddir="cards"
cardtmpl="datacard.tmpl"

#############################
## replace input file name ##
#############################
cp ${carddir}/${cardtmpl} ${datacard}
sed -i "s/INPUT_FILE/${infile}/g" ${datacard}

###########################################
## Ship things over to combine directory ##
###########################################
cp ${indir}/${infile} ${combdir}
cp ${datacard} ${combdir}

#####################
## Now work there! ##
#####################
pushd ${combdir}
eval `scram runtime -sh`

#################################################
## Loop over signals and run asymptotic limits ##
#################################################
for lambda in 100 150 200 250 300 350 400 500 600
do
    for ctau in 0p001 0p1 10 200 400 600 800 1000 1200 10000 
    do
	name=GMSB_L${lambda}TeV_CTau${ctau}cm

	echo "Working on ${name}"

	## make a new datacard for each signal, replacing PDF names
	cp ${datacard} datacard_${name}.txt
	sed -i "s/SIGNAL_PDF/GMSB_L${lambda}_CTau${ctau}_PDF/g" datacard_${name}.txt

	combine -M AsymptoticLimits datacard_${name}.txt --run=expected --name ${name}
    done
done

############
## rename ##
############
rename "higgsCombine" ${outname} *.root
rename ".AsymptoticLimits.mH120" "" *.root

################
## Move back! ##
################
popd
eval `scram runtime -sh`

###########################################
## Ship things over to combine directory ##
###########################################
mkdir -p ${outdir}
cp ${combdir}/${outname}*.root ${outdir}

###################
## Final message ##
###################
echo "Finished ExtractingResults"
