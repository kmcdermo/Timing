#!/bin/bash

## source first
source scripts/common_variables.sh

for cr in qcd gjets
do
    while IFS='' read -r line || [[ -n "${line}" ]]; 
    do
	if [[ ${line} != "" ]];
	then
	    plot=${line}
	    file=${cr}_${plot}.${inTextExt}
	    cp ${crtosrconfigdir}/${cr}.${tmplExt} ${crtosrconfigdir}/${file}
	    sed -i "s/NAME/${plot}/g" ${crtosrconfigdir}/${file}
	fi
    done < ${crtosrconfigdir}/${standardplotlist}.${inTextExt}
done
