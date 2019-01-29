#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
inlimitdir=${1:-"input"}
ws_filename=${2:-"ws_final.root"}
outdir=${3:-"madv2_v3/full_chain/ABCD/combine_input"}
docleanup=${4:-"true"}

## global info
ws_basename="workspace"

#####################################
## Write Results to Datacards + WS ##
#####################################

for lambda in 100 150 200 250 300 350 400 500 600
do
    for ctau in 0p001 0p1 10 200 400 600 800 1000 1200 10000
    do
	## local variables
	sample="GMSB_L${lambda}_CTau${ctau}"
	datacard="${inlimitdir}/${base_datacardABCD}_${sample}.${inTextExt}"
	ws_name="${ws_basename}_${sample}"
	tmplog_file="${sample}.${outTextExt}"

	echo "Working on ${sample}"
	./scripts/writeToWS.sh "${inlimitdir}" "${ws_filename}" "${ws_name}" "${tmplog_file}" "${sample}" "${outdir}"

	## write to datacard: copy template
	cp "${carddir}/${base_datacardABCD}.${tmplExt}" "${datacard}"

	## replace filename
	sed -i "s/INPUT_FILE/${ws_filename}/g" "${datacard}"

	## replace wsname
	sed -i "s/WORK_SPACE/${ws_basename}_${sample}/g" "${datacard}"

	## loop over inputs and replace in datacards
	for input in BIN_A BIN_B BIN_C BIN_D SIG_A SIG_B SIG_C SIG_D
	do 
	    var=$( grep "${input}" "${tmplog_file}" | cut -d " " -f 2 )
	    sed -i "s/${input}/${var}/g" "${datacard}"
	done

	## remove temp log
	if [[ "${docleanup}"  == "true" ]]
	then
	    rm "${tmplog_file}"
	fi
    done
done

########################
## Copy Final WS File ##
########################

## make out dirs
fulldir=${topdir}/${disphodir}/${dir}
PrepOutDir ${fulldir}

## copy log file
cp "${ws_filename}" "${fulldir}"

###################
## Final Message ##
###################

echo "Finished MakingDatacardsAndWSs (ABCD)"
