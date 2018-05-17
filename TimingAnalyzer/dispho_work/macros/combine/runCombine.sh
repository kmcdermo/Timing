#!/bin/bash

## i/o params
infile=${1:-"ws_final.root"}
outdir=${2:-"output"}

## other global vars
combdir="/afs/cern.ch/work/k/kmcdermo/private/dispho/CMSSW_8_1_0/src/HiggsAnalysis/CombinedLimit/working"
datacard="datacard.txt"

#############################
## replace input file name ##
#############################
cp datacard.tmpl ${datacard}
sed -i "s/INPUT_FILE/${infile}/g" ${datacard}

###########################################
## Ship things over to combine directory ##
###########################################
cp ${infile} ${combdir}
cp ${datacard} ${combdir}

#####################
## Now work there! ##
#####################
pushd ${combdir}
eval `scram runtime -sh`

#################################################
## Loop over signals and run asymptotic limits ##
#################################################
for lambda in 100 150 200 250 300 350 400
do
    for ctau in 0p1
    do
	name=GMSB_L${lambda}TeV_CTau${ctau}cm

	echo "Working on ${name}"

	## make a new datacard for each signal, replacing PDF names
	cp ${datacard} datacard_${name}.txt
	sed -i "s/SIGNAL_PDF/GMSB_L${lambda}_CTau${ctau}_PDF/g" datacard_${name}.txt

	combine -M AsymptoticLimits datacard_${name}.txt --run=expected --name asym_${name}
    done
done

##############
## separate ##
##############
name=GMSB_L200TeV_CTau400cm	
echo "Working on ${name}"
cp ${datacard} datacard_${name}.txt
sed -i "s/SIGNAL_PDF/GMSB_L200_CTau400_PDF/g" datacard_${name}.txt
combine -M AsymptoticLimits datacard_${name}.txt --run=expected --name asym_${name} >& asym_${name}.txt

################
## Move back! ##
################
popd
eval `scram runtime -sh`

###########################################
## Ship things over to combine directory ##
###########################################
mkdir -p ${outdir}
cp ${combdir}/higgsCombine*.root ${outdir}

##############
## clean-up ##
##############
rm ${datacard}
