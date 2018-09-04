#!/bin/bash

## source first
source scripts/common_variables.sh

## command line options
outdir=${1:-"plots/ntuples_v4/checks_v3/DEG_test/srplots_wgt"}
plotlist=${2:-"standard"}
docleanup=${3:-"true"}

## make tmp director for configs
tmpdir="${srplotconfigdir}/tmp"
mkdir -p "${tmpdir}"

## loop over plots to make input plots for SRPlotter, then make SRPlot
while IFS='' read -r plot || [[ -n "${plot}" ]]
do
    if [[ ${plot} != "" ]]
    then
	echo "Working on plot: ${plot}"
	
        ## make tmp config file for srplotter
	tmpconfig="${tmpdir}/${plot}.${inTextExt}"
	> "${tmpconfig}"

	## fill plot to use
	echo "${plotconfigdir}=${plotconfigdir}/${plot}.${inTextExt}" >> "${tmpconfig}"
    
        ## loop over regions to make input plots
	for input in "${inputs[@]}"
	do 
	    echo ${!input} | while read -r label infile insigfile sel varwgtmap
	    do
		echo "Creating input plot for: ${input}"

		## output filename
		outfile="${plot}_${label}"
		
		## determine which misc file to use
		misc=$(GetMisc ${input} ${plot})
	
		## make the plot (with variable weights applied as needed)
		./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${varwgtconfigdir}/${varwgtmap}.${inTextExt}" "${plotconfigdir}/${plot}.${inTextExt}" "${miscconfigdir}/${misc}.${inTextExt}" "${MainEra}" "${outfile}" "${outdir}/${plot}"

		## use output to make config files for next step
		echo "${input}_in=${outfile}.root" >> "${tmpconfig}"

	    done ## end loop over reading of input
	done ## end loop over inputs array

        ## determine which misc
	misc=$(GetMisc ${input} ${plot})

        ## Now run SRPlotter over input files
	./scripts/runSRPlotter.sh "${tmpconfig}" "${miscconfigdir}/${misc}.${inTextExt}" "${MainEra}" "${plot}_SRPlot" "${outdir}/${plot}"
	
    fi ## end check on plot is valid
done < "${srplotconfigdir}/${plotlist}.${inTextExt}" ## end loop over plots

## delete tmpdir
if [[ "${docleanup}" == "true" ]]; then
    echo "Removing tmp dir: ${tmpdir}"
    rm -r "${tmpdir}"
fi

## Final message
echo "Finished MakingPlotsForSR"
