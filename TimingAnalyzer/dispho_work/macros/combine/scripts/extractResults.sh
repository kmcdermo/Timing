#!/bin/bash

## i/o params
infile=${1:-"ws_final.root"}
outname=${2-"AsymLim"}
outdir=${3:-"output"}
indir="input"

## other global vars
combdir="/afs/cern.ch/work/k/kmcdermo/private/dispho/CMSSW_8_1_0/src/HiggsAnalysis/CombinedLimit/working"
datacard="datacard.txt"

#############################
## replace input file name ##
#############################
cp ${indir}/datacard.tmpl ${datacard}
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
for lambda in 100 150 200 250 300 350 400
do
    for ctau in 0p1 10 600 1200
    do
	name=GMSB_L${lambda}TeV_CTau${ctau}cm

	echo "Working on ${name}"

	## make a new datacard for each signal, replacing PDF names
	cp ${datacard} datacard_${name}.txt
	sed -i "s/SIGNAL_PDF/GMSB_L${lambda}_CTau${ctau}_PDF/g" datacard_${name}.txt

	combine -M AsymptoticLimits datacard_${name}.txt --run=expected --name ${name}
    done
done

##############
## separate ##
##############
name=GMSB_L200TeV_CTau400cm	
echo "Working on ${name}"
cp ${datacard} datacard_${name}.txt
sed -i "s/SIGNAL_PDF/GMSB_L200_CTau400_PDF/g" datacard_${name}.txt
combine -M AsymptoticLimits datacard_${name}.txt --run=expected --name ${name}

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

##############
## clean-up ##
##############
rm ${combdir}/*.txt ${combdir}/*.root
rm ${datacard}
