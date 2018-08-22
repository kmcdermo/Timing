#!/bin/bash

##################
## Source First ##
##################
source scripts/common_variables.sh

############
## Config ##
############

## command line inputs
outdir=${1:-"plots/ntuples_v4/checks_v4/era_plots"}
plot=${2:-"met_zoom"}
usewgts=${3:-"true"}

## eras
declare -a eras=(2017B 2017C 2017D 2017E 2017F)

################
## Run Script ##
################

## loop over all inputs
for input in "${inputs[@]}"
do 
    echo ${!input} | while read -r label infile insigfile sel varwgtmap
    do
	###########################
	## Check For Use VarWgts ##
	###########################
	if [[ "${usewgts}" == "false" ]]; then
	    varwgtmap="empty"
	fi
	
	##############################
	## Define Base Outfile Text ##
	##############################
	baseoutfile="${plot}_${label}"

	###########################
	## Setup Era Plot Config ##
	###########################
	eraplotconfig="tmp_era_config.${inTextExt}"
	> "${eraplotconfig}"
	echo "in_file_text=${baseoutfile}" >> "${eraplotconfig}"
	echo -n "eras=" >> "${eraplotconfig}"

	####################
	## Loop Over Eras ##
	####################
	for era in "${eras[@]}"
	do
	    ## output filename
	    outfile="${baseoutfile}_${era}"
	    
	    ## determine which misc file to use
	    misc=$(GetMisc ${input} ${plot})

	    ## make plot for each era
	    ./scripts/runEraTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plotconfigdir}/${plot}.${inTextExt}" "${miscconfigdir}/${misc}.${inTextExt}" "${outfile}" "${era}" "${outdir}/${label}"

	    ## record the era in config
	    echo -n " ${era}" >> "${eraplotconfig}"
	done

	########################################
	## Make Full Lumi Plot For Comparison ##
	########################################
	
        ## output filename
	outfile="${baseoutfile}"

	## determine which misc file to use
	misc=$(GetMisc ${input} ${plot})

        ## run standard tree plotting script
	./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plotconfigdir}/${plot}.${inTextExt}" "${miscconfigdir}/${misc}.${inTextExt}" "${outfile}_full_lumi" "${outdir}/${label}"

	##################
	## Compare Eras ##
	##################
	./scripts/runEraPlotter.sh "${eraplotconfig}" "${plotconfigdir}/${plot}.${inTextExt}" "${baseoutfile}_era_comp" "${outdir}/${label}"

	#############
	## Cleanup ##
	#############
	rm "${eraplotconfig}"

    done ## end do over read of input
done ## end loop over inputs
