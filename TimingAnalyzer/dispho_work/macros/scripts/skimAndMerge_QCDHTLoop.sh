#!/bin/bash

source common_variables.sh

## input
label=${1}

for bin in 100to200 200to300 300to500 500to700 700to1000 1000to1500 1500to2000 2000toInf
do
    ## global vars
    mcbase="QCD_HT"
    tmptxt="${mcbase}${bin}_files.txt"

    ## directories needed
    indir="${eosbase}/${mcbase}${bin}_${bkgdtune}/${label}_${mcbase}${bin}_${bkgdtune}"
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
