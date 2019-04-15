#!/bin/bash

## source first
source scripts/common_variables.sh

## config
outdir=${1:-"ntuples_v4/checks_v2"}
plotlist=${2:-"${reducedplotlist}"}
save_meta_data=${3:-0}

## inner loop 
function make1DplotsSubRoutine
{
    ## derived config from input
    local input=${1}
    local label=${2}
    local infile=${3}
    local insigfile=${4}
    local sel=${5}

    ## loop over plots
    while IFS='' read -r plot || [[ -n "${plot}" ]];
    do
	if [[ ${plot} != "" ]];
	then
	    ## output filename
	    local outfile="${plot}_${label}"
	
	    ## determine which misc file to use
	    local misc=$( GetMisc ${input} ${plot} )
	
	    ## run script
	    echo "Making plot: ${plot}"
	    ./scripts/runTreePlotter.sh "${skimdir}/${infile}.root" "${skimdir}/${insigfile}.root" "${cutconfigdir}/${sel}.${inTextExt}" "${plotconfigdir}/${plot}.${inTextExt}" "${miscconfigdir}/${misc}.${inTextExt}" "${MainEra}" ${save_meta_data} "${outfile}" "${outdir}/${label}" &

	    ## wait until current plot is finished before moving on!
	    wait
	fi
    done < "${plotconfigdir}/${plotlist}.${inTextExt}"
}
export -f make1DplotsSubRoutine

## main loops
for input in "${inputs[@]}"
do 
    echo ${!input} | while read -r label infile insigfile sel
    do
	echo "Making 1D plots for: ${label}"
	make1DplotsSubRoutine "${input}" "${label}" "${infile}" "${insigfile}" "${sel}" >& "${input}_1Dplots.log" &
    done
done

## wait for everything to finish!
echo "Waiting for all 1D plots to finish running..."
wait

## final message
echo "Finished Making1DPlots"
