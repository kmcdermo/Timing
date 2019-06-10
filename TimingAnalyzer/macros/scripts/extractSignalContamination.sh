#!/bin/bash

outfile="sig_contamination.txt"
> "${outfile}"

for lambda in 100 200 300
do
    for ctau in 10 200 1000
    do
	signal="GMSB_L${lambda}_CTau${ctau}"
	output="${lambda}, ${ctau}"

	for category in exclusive_1pho inclusive_2pho
	do
	    for var in X Y
	    do
		line=$( grep -w "${signal}" "met_vs_time_templates_${category}_proj${var}.log" )
		ratio=$( echo "${line}" | cut -d " " -f 2 )
		err=$( echo "${line}" | cut -d " " -f 4 )

		output+=" & \$${ratio} \\pm ${err}\$"
	    done
	done

	output+=" \\\\"
	echo "${output}" >> "${outfile}"
    done
done