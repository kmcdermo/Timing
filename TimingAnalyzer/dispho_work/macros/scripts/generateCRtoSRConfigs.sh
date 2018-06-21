#!/bin/bash

for cr in qcd gjets
do
    while IFS='' read -r line || [[ -n "${line}" ]]; 
    do
	if [[ ${line} != "" ]];
	then
	    plot=${line}
	    file=${cr}_${plot}.txt
	    cp crtosr_config/${cr}.tmpl crtosr_config/${file}
	    sed -i "s/NAME/${plot}/g" crtosr_config/${file}
	fi
    done < crtosr_config/standard_plots.txt
done
