#!/bin/bash

##################
## Source First ##
##################

source scripts/common_variables.sh

###################
## Configuration ##
###################

## Command Line Input
tmplimitdir=${1:-"tmp"}
inplotsfile=${2:-"met_vs_time_box_SR.root"}
inlimitdir=${3:-"input"}

#####################################
## Write Results to Datacards + WS ##
#####################################

for lambda in 100 150 200 250 300 350 400 500 600
do
    for ctau in 0p001 0p1 10 200 400 600 800 1000 1200 10000 
    do
	## local variables
	name="GMSB_L${lambda}_CTau${ctau}"
	tmpdatacard="${inlimitdir}/${base_datacardABCD}_${name}.${inTextExt}"
	tmpwsfile="${base_wsfileABCD}_${name}.root"
	tmpoutfile="${name}.${inTextExt}"

	echo "Working on ${name}"
	./scripts/writeToWS.sh "${tmplimitdir}" "${inplotsfile}" "${name}" "${tmpwsfile}"

	## write to datacard: copy template
	cp "${carddir}/${base_datacardABCD}.${tmplExt}" "${tmpdatacard}"

	## replace filename
	sed -i "s/INPUT_FILE/${tmpwsfile}/g" "${tmpdatacard}"

	## loop over inputs and replace in datacards
	for input in BIN_A BIN_B BIN_C BIN_D SIG_A SIG_B SIG_C SIG_D
	do 
	    var=$( grep "${input}" "${tmpoutfile}" | cut -d " " -f 2 )
	    sed -i "s/${input}/${var}/g" "${tmpdatacard}"
	done

	## move WS file
	mv "${tmpwsfile}" "${inlimitdir}"

	## remove temp outfile
	rm "${tmpoutfile}"
    done
done

