#!/bin/bash

source common_variables.sh

## input
label=${1}

for subera in "B v1" "B v2" "C v1" "C v2" "C v3" "D v1" "E v1" "F v1"
do echo ${subera} | while read -r era version
    do
        ## global vars
	dataset="SinglePhoton"
	database="Run2017${era}-PromptReco-${version}"
	tmptxt="${dataset}${era}_${version}_files.txt"

        ## directories needed
	indir="${eosbase}/${dataset}/${label}_${database}"
	tmpdir="${tmpbase}/${dataset}/${era}/${version}"
	outdir="${outbase}/Data/${dataset}/${era}/{version}"
	
        ## get timestamp
	timestamp=$(eos ls ${indir})

        ## do the real work 
	echo "Processing: " ${dataset} ${era} ${version}
    
        ## make tmp dir
	mkdir -p ${tmpdir}
    
        ## first get list of files
	./getListOfFiles.sh ${indir} ${tmptxt}
	
        ## Skim and save on tmp dir, move back to EOS
	./skimAndMerge.sh "${rootbase}/${indir}/${timestamp}/0000" "${tmpdir}" "${outdir}" "${tmptxt}"
	
	rm ${tmptxt}
    done
done
