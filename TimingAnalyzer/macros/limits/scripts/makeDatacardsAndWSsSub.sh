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
ws_filename=${2:-"ws_inputs.root"}
ws_basename=${3:-"workspace"}
ws_outfile_base=${4:-"ws"}
lambda=${5:-"100"}
docleanup=${6:-"true"}

## derived config
ws_outfile_name="${ws_outfile_base}_L${lambda}.root"

#####################################
## Write Results to Datacards + WS ##
#####################################

for ctau in 0p001 0p1 10 200 400 600 800 1000 1200
do
    ## local variables
    sample="GMSB_L${lambda}_CTau${ctau}"
    datacard="${inlimitdir}/${base_datacardABCD}_${sample}.${inTextExt}"
    ws_name="${ws_basename}_${sample}"
    tmplog_file="${sample}_input.${outTextExt}"
    
    echo "Working on ${sample}"
    ./scripts/writeToWS.sh "${inlimitdir}" "${ws_filename}" "${ws_name}" "${sample}" "${tmplog_file}" "${ws_outfile_name}"

    ## write to datacard: copy template
    cp "${carddir}/${base_datacardABCD}.${tmplExt}" "${datacard}"
    
    ## replace filename
    sed -i "s/INPUT_FILE/${ws_outfile_name}/g" "${datacard}"
    
	## replace wsname
    sed -i "s/WORK_SPACE/${ws_basename}_${sample}/g" "${datacard}"
    
    ## loop over inputs and replace in datacards
    for region in A B C D #E F G H I
    do
	for input in BIN SIG
	do 
	    var="${input}_${region}"
	    val=$( grep "${var}" "${tmplog_file}" | cut -d " " -f 2 )
	    sed -i "s/${var}/${val}/g" "${datacard}"
	done
    done
    
    if [[ "${docleanup}" == "true" ]]
    then
	## rm log gile
	rm "${tmplog_file}"
    fi
done

