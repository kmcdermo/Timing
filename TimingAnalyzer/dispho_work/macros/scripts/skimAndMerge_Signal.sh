#!/bin/bash

source common_variables.sh

## input
label=${1}

for gridpoint in L200TeV_CTau400cm_930
do
    ## global vars
    mcbase="GMSB"
    tmptxt="${mcbase}${gridpoint}_files.txt"

    ## directories needed
    indir="${eosbase}/${mcbase}_${gridpoint}/${label}_${mcbase}_${gridpoint}"
    tmpdir="${tmpbase}/${mcbase}/${gridpoint}"
    outdir="${outbase}/MC/${mcbase}/${gridpoint}"

    ## get timestamp
    timestamp=$(eos ls ${indir})

    ## do the real work 
    echo "Processing: " ${mcbase} ${gridpoint}
    
    ## make tmp dir
    mkdir -p ${tmpdir}
    
    ## first get list of files
    ./getListOfFiles.sh ${indir} ${tmptxt}

    ## Skim and save on tmp dir, move back to EOS
    ./skimAndMerge.sh "${rootbase}/${indir}/${timestamp}/0000" "${tmpdir}" "${outdir}" "${tmptxt}"

    rm ${tmptxt}
done
