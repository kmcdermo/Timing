#!/bin/bash

samples=${1}
files=${2}

while IFS='' read -r line || [[ -n "${line}" ]]; do
    echo $line
    indir=$(echo ${line} | cut -d " " -f 1)
    wgtfile=$(echo ${line} | cut -d " " -f 2)
    ./scripts/runSumWeights.sh ${indir} ${files} ${wgtfile} ""
done < "${samples}"
