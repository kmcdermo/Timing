#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Config
x_log=${1:-"xs.${outTextExt}"}
y_log=${2:-"ys.${outTextExt}"}
scan_log=${3:-"scan_for_best_r.${outTextExt}"}
outlimitplotdir=${4:-"limits"}
combinelogname=${5:-"combine"}
outdir=${6:-"madv2_v3/full_chain/results_ABCD"}

## Max ratio between observed and expected
max_ratio=20

## Derived Config
indir="${topdir}/${disphodir}/${outdir}"

####################
## Read In Arrays ##
####################

mapfile -t xs < "${x_log}"
mapfile -t ys < "${y_log}"

#################################
## Function For Each Gridpoint ##
#################################

function GetBestDir ()
{
    local sample=${1}

    local best_dir=""
    local best_exp_r="10000"

    for x in "${xs[@]}"
    do 
	while read -r xbin xblind
	    do
		for y in "${ys[@]}"
		do 
		    while read -r ybin yblind
		    do
			local dir="x_${xbin}_y_${ybin}"
			local infile="${indir}/${dir}/${outlimitplotdir}/${combinelogname}_${sample}.${outTextExt}"
			
			local obs_r=$( grep "Observed Limit" "${infile}" | cut -d " " -f 5 )
			local exp_r=$( grep "Expected 50.0%" "${infile}" | cut -d " " -f 5 )
			
			## ensure it is not empty
			if [[ "${obs_r}" == "" ]] || [[ "${exp_r}" == "" ]]
			then
			    continue
			fi
			
			## check for sensible results
			local ratio=$( echo "${exp_r} / ${obs_r}" | bc -l )
			if (( $( echo "${ratio} > ${max_ratio}" | bc -l ) )) || (( $( echo "${ratio} < 1/${max_ratio}" | bc -l ) ))
			then
			    continue
			fi

			## check for best r
			if (( $( echo "${exp_r} < ${best_exp_r}" | bc -l ) ))
			then
			    local best_exp_r="${exp_r}"
			    local best_dir="${dir}"
			fi
		    done <<< "$( echo ${y} )"
		done ## end loop over ys
	done <<< "$( echo ${x} )"
    done ## end loop over xs
    
    ## echo best result
    echo "${best_dir}"
}

############################################
## Loop Over All Gridpoints To Get Best R ##
############################################

for lambda in 100 150 200 250 300 350 400
do
    for ctau in 0p001 0p1 10 200 400 600 800 1000 1200 10000
    do
	sample="GMSB_L${lambda}_CTau${ctau}"

	echo "Working on ${sample}"
	best_dir=$( GetBestDir "${sample}" )

	echo "${sample}" "${indir}/${best_dir}" >> "${scan_log}"
    done ## end loop over ctau
done ## end loop over lambda

#########################
## Copy Logs To Output ##
#########################

## make out dirs
fulldir="${topdir}/${disphodir}/${outdir}/${outlimitplotdir}"
PrepOutDir "${fulldir}"

## copy log files
cp "${scan_log}" "${x_log}" "${y_log}" "${fulldir}"

###################
## Final Message ##
###################

echo "Finished scanning for best limits"
