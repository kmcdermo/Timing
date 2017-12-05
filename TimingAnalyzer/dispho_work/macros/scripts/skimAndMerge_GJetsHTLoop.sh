#!/bin/bash

source common_variables.sh

## input
label=${1}

for bin in 40To100 100To200 200To400 400To600 600ToInf
do
    ## global vars
    mcbase="GJets_HT"
    tmptxt="${mcbase}${bin}_files.txt"

    ## directories needed
    indir="${eosbase}/${mcbase}-${bin}_${bkgdtune}/${label}_${mcbase}-${bin}_${bkgdtune}"
    tmpdir="${tmpbase}/${mcbase}/${bin}"
    outdir="${outbase}/MC/${mcbase}/${bin}"

    ## get timestamp
    timestamp=$(eos ls ${indir})

    ## do the real work 
    echo "Processing: " ${mcbase} ${bin}
    
    ## make tmp dir
    mkdir -p ${tmpdir}
    
    ## first get list of files
    ./getListOfFiles.sh ${indir} ${tmptxt}

    ## Skim and save on tmp dir, move back to EOS
    ./skimAndMerge.sh "${rootbase}/${indir}/${timestamp}/0000" "${tmpdir}" "${outdir}" "${tmptxt}"

    rm ${tmptxt}
done
