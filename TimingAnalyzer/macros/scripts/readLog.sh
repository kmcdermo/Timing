#!/usr/local/bin/bash

input=${1}
output=${2}

> "${output}"
for ctau in 10 50 100 200 400 600 800 1000 1200 10000
do
    string="${ctau}"
    for lambda in 100 150 200 250 300 350 400
    do
	line=$( grep -w "^${lambda} ${ctau}" "${input}" )
	x=$( echo "${line}" | cut -d " " -f 3 )
	y=$( echo "${line}" | cut -d " " -f 4 )
	
	string+=" ${x},${y}"
    done
    echo "${string}" >> "${output}"
done
