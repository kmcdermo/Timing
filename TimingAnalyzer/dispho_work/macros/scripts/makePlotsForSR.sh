#!/bin/bash

## command line options
outdir=${1:-"plots/ntuples_v4/checks_v3/srplots"}

## input configs
CR_GJets="gjets signals_gjets always_true gjets_phopt_0_map cr_gjets_DEG"
CR_QCD="qcd signals_qcd cuts_v3/invertiso0_v0 qcd_phopt_0_map cr_qcd_DEG"
SR="sr signals_sr always_true empty sr_SPH"
declare -a inputs=(CR_GJets CR_QCD SR)

## make tmp director for configs
tmpdir="srplot_config/tmp"
mkdir -p "${tmpdir}"

## loop over plots to make input plots for SRPlotter, then make SRPlot
while IFS='' read -r plot || [[ -n "${plot}" ]]
do
    if [[ ${plot} != "" ]]
    then
	echo "Working on plot: ${plot}"
	
        ## make tmp config file for srplotter
	tmpconfig="${tmpdir}/${plot}.txt"
	echo "plot_config=plot_config/${plot}.txt" >> "${tmpconfig}"
    
        ## loop over regions to make input plots
	for input in "${inputs[@]}"
	do 
	    echo ${!input} | while read -r infile insigfile sel varwgtmap label
	    do
		echo "Creating input plot for: ${input}"

		## output filename
		outfile="${plot}_${label}"
		
		## determine which misc file to use
		misc="misc"
		if [[ ${input} == "SR" ]] 
		then
		    if [[ ${plot} == *"met"* ]] || [[ ${plot} == *"phoseedtime_0"* ]] 
		    then
			misc="misc_blind"
		    fi
		fi

		## make the plot (with variable weights applied as needed)
		./scripts/runTreePlotter.sh "skims/${infile}.root" "skims/${insigfile}.root" "cut_config/${sel}.txt" "varwgt_config/${varwgtmap}.txt" "plot_config/${plot}.txt" "misc_config/${misc}.txt" "${outfile}" "${outdir}/${plot}"

		## use output to make config files for next step
		echo "${input}_in=${outfile}.root" >> "${tmpconfig}"

	    done ## end loop over reading of input
	done ## end loop over inputs array

        ## determine which 
	misc="misc"
	if [[ ${plot} == *"met"* ]] || [[ ${plot} == *"phoseedtime_0"* ]]
	then
	    misc="misc_blind"
	fi
	
        ## Now run SRPlotter over input files
	./scripts/runSRPlotter.sh "${tmpconfig}" "misc_config/${misc}.txt" "${plot}_SRPlot" "${outdir}/${plot}"
	
    fi ## end check on plot is valid
done < srplot_config/standard_plots.txt ## end loop over plots

## delete tmpdir
echo "Removing tmp dir: ${tmpdir}"
rm -r ${tmpdir}

## Final message
echo "Finished MakingPlotsForSR"
