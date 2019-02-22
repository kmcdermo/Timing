#!/bin/bash

## inputs
indir=${1:-"/eos/user/k/kmcdermo/www/dispho/plots/madv2_v3/checks_v16"}

declare -a xs=("-1_0_1_2_5 5" "-1_0_1_2_3 3" "0_1_2_3_5 3" "0_2_3_5_10 3" "-0.5_1_2_5_10 5" "-0.5_1.5_2_5_10 5" "-1_1_2_5_10 5" "-1_1_2_3_5 3")
declare -a ys=("100_200_300_500 200" "50_100_200_300 200" "100_300_500_1000 300" "50_150_300_500 300" "100_200_500_1000 200" "50_200_500_1000 200" "50_100_150_200 200")

## outputs
outfile=${2:-"abcd_scan_for_r.log"}
> "${outfile}"

## function
function GetBestDir ()
{
    local sample=${1}

    local best_dir=""
    local best_r="1000"

    for x in "${xs[@]}"
    do 
	while read -r xbin xblind
	    do
		for y in "${ys[@]}"
		do 
		    while read -r ybin yblind
		    do
			local dir="x_${xbin}_y_${ybin}"
			local infile="${indir}/${dir}/limits/combine_${sample}.log"
			
			local r=$( grep "Expected 50.0%" "${infile}" | cut -d " " -f 5 )
			
			if [[ "${r}" == "" ]]
			then
			    continue
			fi
			
			## check for best r
			if (( $( echo "${r} < ${best_r}" | bc -l ) ))
			then
			    local best_r="${r}"
			    local best_dir="${dir}"
			fi
		    done <<< "$( echo ${y} )"
		done ## end loop over ys
	done <<< "$( echo ${x} )"
    done ## end loop over xs
    
    ## echo best result
    echo "${best_dir}"
}

## actual loop
for lambda in 100 150 200 250 300 350 400 500 600
do
    for ctau in 0p001 0p1 10 200 400 600 800 1000 1200
    do
	sample="GMSB_L${lambda}_CTau${ctau}"

	echo "Working on ${sample}"
	best_dir=$( GetBestDir "${sample}" )

	echo "${sample}" "${indir}/${best_dir}" >> "${outfile}"
    done ## end loop over ctau
done ## end loop over lambda

