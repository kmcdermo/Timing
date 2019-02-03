#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"ntuples_v4/checks_v2"}

misc="misc_zee_effseedE20"
plotlist="zee_effseedE20"
effseedE_ranges=("15 25" "15 17" "17 19" "19 21" "21 23" "23 25")
min=15
max=25

## main loops
for input in "${inputs[@]}"
do 
    echo ${!input} | while read -r input_label infile insigfile sel varwgtmap
    do
	## loop over plots
	while IFS='' read -r plot || [[ -n "${plot}" ]];
	do
	    if [[ ${plot} != "" ]];
	    then
		## overlay plots
		overlay="tmp_list_of_files.txt"
		> "${overlay}"

		for effseedE_range in "${effseedE_ranges[@]}"
		do
		    echo ${effseedE_range} | while read -r effseedE_low effseedE_up
		    do
			## write cuts
			cut="tmp_cut_config.txt"
			> "${cut}"

			echo "common_cut=((((phoseedE_0*phoseedE_1)/(sqrt(pow(phoseedE_0,2)+pow(phoseedE_1,2))))>=${effseedE_low})&&(((phoseedE_0*phoseedE_1)/(sqrt(pow(phoseedE_0,2)+pow(phoseedE_1,2))))<${effseedE_up}))" >> "${cut}"
			echo "data_cut=" >> "${cut}"
			echo "bkgd_cut=" >> "${cut}"
			echo "sign_cut=" >> "${cut}"

			## label
			label="${effseedE_low}_to_${effseedE_up}"

	  	        ## output filename
			outfile="${plot}_${label}"
			
		        ## run script
			./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cut}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plotconfigdir}/${plot}.${inTextExt}" "${miscconfigdir}/${misc}.${inTextExt}" "${MainEra}" "${outfile}" "${outdir}/effseedE_${label}"

			## rm cut file
			rm "${cut}"

			## write out
			if [[ "${effseedE_low}" != "${min}" ]] || [[ "${effseedE_up}" != "${max}" ]]
			then
			    echo "${outfile}.root" "${label}"  >> "${overlay}"
			fi

		    done ## end read over effseedE_range
		done ## end loop over effseedE_ranges

          	## run overplotter
		./test_macros/scripts/overplot_dataplots.sh "${overlay}" "${plotconfigdir}/${plot}.${inTextExt}" "${plot}_comp" "${outdir}"

	        ## rm tmp file
		rm "${overlay}"

	    fi ## end check over plot list is valid
	done < "${plotconfigdir}/${plotlist}.${inTextExt}" ## end loop over plot list
    done ## end read of inputs
done ## end loop over inputs

## Final message
echo "Finished Making1DPlots"
